#include "hubflow/platform/mysql/mySqlConnection.hpp"

#include <chrono>
#include <mysql.h>
#include <thread>
#include <utility>

namespace hubflow::platform
{
    namespace
    {
        constexpr std::size_t COLUMN_BUFFER_SIZE {4096};
        constexpr auto RETRY_DELAY {std::chrono::seconds {2}};

        struct StatementCloser
        {
            void operator()(MYSQL_STMT* pStatement) const noexcept
            {
                mysql_stmt_close(pStatement);
            }
        };

        using StatementHandle = std::unique_ptr<MYSQL_STMT, StatementCloser>;

        struct ParameterStorage
        {
            std::vector<MYSQL_BIND> binds;
            std::vector<std::string> texts;
            std::vector<std::int64_t> signedNumbers;
            std::vector<std::uint64_t> unsignedNumbers;
            std::vector<double> decimals;
            std::vector<unsigned long> lengths;
        };

        void bindParameters(MYSQL_STMT* pStatement, const std::vector<SqlValue>& parameters, ParameterStorage& storage)
        {
            const auto count = parameters.size();
            storage.binds.assign(count, MYSQL_BIND {});
            storage.texts.reserve(count);
            storage.signedNumbers.reserve(count);
            storage.unsignedNumbers.reserve(count);
            storage.decimals.reserve(count);
            storage.lengths.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
            {
                MYSQL_BIND& bind = storage.binds[i];
                if (const auto* pText = std::get_if<std::string>(&parameters[i]))
                {
                    storage.texts.push_back(*pText);
                    storage.lengths.push_back(pText->size());
                    bind.buffer_type = MYSQL_TYPE_STRING;
                    bind.buffer = storage.texts.back().data();
                    bind.buffer_length = storage.texts.back().size();
                    bind.length = &storage.lengths.back();
                }
                else if (const auto* pSigned = std::get_if<std::int64_t>(&parameters[i]))
                {
                    storage.signedNumbers.push_back(*pSigned);
                    bind.buffer_type = MYSQL_TYPE_LONGLONG;
                    bind.buffer = &storage.signedNumbers.back();
                }
                else if (const auto* pUnsigned = std::get_if<std::uint64_t>(&parameters[i]))
                {
                    storage.unsignedNumbers.push_back(*pUnsigned);
                    bind.buffer_type = MYSQL_TYPE_LONGLONG;
                    bind.buffer = &storage.unsignedNumbers.back();
                    bind.is_unsigned = true;
                }
                else if (const auto* pDecimal = std::get_if<double>(&parameters[i]))
                {
                    storage.decimals.push_back(*pDecimal);
                    bind.buffer_type = MYSQL_TYPE_DOUBLE;
                    bind.buffer = &storage.decimals.back();
                }
                else
                {
                    bind.buffer_type = MYSQL_TYPE_NULL;
                }
            }
            if (count > 0 && mysql_stmt_bind_param(pStatement, storage.binds.data()) != 0)
            {
                throw database_error {mysql_stmt_error(pStatement)};
            }
        }

        StatementHandle prepare(MYSQL* pHandle, const std::string& sql)
        {
            StatementHandle statement {mysql_stmt_init(pHandle)};
            if (!statement || mysql_stmt_prepare(statement.get(), sql.c_str(), sql.size()) != 0)
            {
                throw database_error {statement ? mysql_stmt_error(statement.get()) : mysql_error(pHandle)};
            }
            return statement;
        }

        void run(MYSQL_STMT* pStatement, const std::vector<SqlValue>& parameters)
        {
            ParameterStorage storage;
            bindParameters(pStatement, parameters, storage);
            if (mysql_stmt_execute(pStatement) != 0)
            {
                throw database_error {mysql_stmt_error(pStatement)};
            }
        }

        std::vector<SqlRow> fetchAll(MYSQL_STMT* pStatement)
        {
            std::vector<SqlRow> rows;
            MYSQL_RES* pMetadata = mysql_stmt_result_metadata(pStatement);
            if (pMetadata != nullptr)
            {
                const unsigned int columns {mysql_num_fields(pMetadata)};
                std::vector<MYSQL_BIND> binds(columns);
                std::vector<std::vector<char>> buffers(columns, std::vector<char>(COLUMN_BUFFER_SIZE));
                std::vector<unsigned long> lengths(columns);
                auto spNulls = std::make_unique<bool[]>(columns);
                for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex)
                {
                    binds[columnIndex].buffer_type = MYSQL_TYPE_STRING;
                    binds[columnIndex].buffer = buffers[columnIndex].data();
                    binds[columnIndex].buffer_length = COLUMN_BUFFER_SIZE;
                    binds[columnIndex].length = &lengths[columnIndex];
                    binds[columnIndex].is_null = &spNulls[columnIndex];
                }
                mysql_free_result(pMetadata);
                if (mysql_stmt_bind_result(pStatement, binds.data()) != 0 || mysql_stmt_store_result(pStatement) != 0)
                {
                    throw database_error {mysql_stmt_error(pStatement)};
                }
                for (int status = mysql_stmt_fetch(pStatement); status == 0 || status == MYSQL_DATA_TRUNCATED;
                     status = mysql_stmt_fetch(pStatement))
                {
                    SqlRow row;
                    row.reserve(columns);
                    for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex)
                    {
                        row.emplace_back(spNulls[columnIndex]
                                             ? std::nullopt
                                             : std::optional<std::string> {
                                                   std::string {buffers[columnIndex].data(), lengths[columnIndex]}});
                    }
                    rows.emplace_back(std::move(row));
                }
            }
            return rows;
        }
    } // namespace

    void MySqlConnection::HandleCloser::operator()(MYSQL* pHandle) const noexcept
    {
        mysql_close(pHandle);
    }

    MySqlConnection::MySqlConnection(DatabaseSettings settings)
        : m_settings {std::move(settings)}
    {
        connect();
    }

    MySqlConnection::~MySqlConnection() = default;

    void MySqlConnection::connect()
    {
        std::string lastError {"sin intentos"};
        for (unsigned attempt = 0; attempt < m_settings.connectRetries && !m_spHandle; ++attempt)
        {
            std::unique_ptr<MYSQL, HandleCloser> spHandle {mysql_init(nullptr)};
            const unsigned int timeoutSeconds {5};
            mysql_options(spHandle.get(), MYSQL_OPT_CONNECT_TIMEOUT, &timeoutSeconds);
            mysql_options(spHandle.get(), MYSQL_SET_CHARSET_NAME, "utf8mb4");
            if (mysql_real_connect(spHandle.get(),
                                   m_settings.host.c_str(),
                                   m_settings.user.c_str(),
                                   m_settings.password.c_str(),
                                   m_settings.schema.c_str(),
                                   m_settings.port,
                                   nullptr,
                                   CLIENT_MULTI_STATEMENTS) != nullptr)
            {
                m_spHandle = std::move(spHandle);
            }
            else
            {
                lastError = mysql_error(spHandle.get());
                std::this_thread::sleep_for(RETRY_DELAY);
            }
        }
        if (!m_spHandle)
        {
            throw database_error {"No se pudo conectar a MySQL: " + lastError};
        }
    }

    void MySqlConnection::ensureConnected()
    {
        if (mysql_ping(m_spHandle.get()) != 0)
        {
            m_spHandle.reset();
            connect();
        }
    }

    std::uint64_t MySqlConnection::execute(const std::string& sql, const std::vector<SqlValue>& parameters)
    {
        ensureConnected();
        const StatementHandle statement {prepare(m_spHandle.get(), sql)};
        run(statement.get(), parameters);
        return mysql_stmt_affected_rows(statement.get());
    }

    std::vector<SqlRow> MySqlConnection::query(const std::string& sql, const std::vector<SqlValue>& parameters)
    {
        ensureConnected();
        const StatementHandle statement {prepare(m_spHandle.get(), sql)};
        run(statement.get(), parameters);
        return fetchAll(statement.get());
    }

    void MySqlConnection::executeScript(const std::string& script)
    {
        ensureConnected();
        if (mysql_real_query(m_spHandle.get(), script.c_str(), script.size()) != 0)
        {
            throw database_error {mysql_error(m_spHandle.get())};
        }
        for (int status = 0; status == 0; status = mysql_next_result(m_spHandle.get()))
        {
            MYSQL_RES* pResult = mysql_store_result(m_spHandle.get());
            if (pResult != nullptr)
            {
                mysql_free_result(pResult);
            }
        }
        if (mysql_errno(m_spHandle.get()) != 0)
        {
            throw database_error {mysql_error(m_spHandle.get())};
        }
    }

} // namespace hubflow::platform
