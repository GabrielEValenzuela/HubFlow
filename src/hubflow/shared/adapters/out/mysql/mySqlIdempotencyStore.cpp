#include "hubflow/shared/adapters/out/mysql/mySqlIdempotencyStore.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"

namespace hubflow::shared::adapters
{
    namespace
    {
        using platform::SqlColumns;

        constexpr const char* SELECT_RESPONSE {R"SQL(
    SELECT method, path, status, body FROM idempotency_keys WHERE user_id = ? AND idem_key = ?
)SQL"};

        constexpr const char* INSERT_RESPONSE {R"SQL(
    INSERT IGNORE INTO idempotency_keys (user_id, idem_key, method, path, status, body) VALUES (?, ?, ?, ?, ?, ?)
)SQL"};
    } // namespace

    MySqlIdempotencyStore::MySqlIdempotencyStore(platform::MySqlConnection& connection)
        : m_connection {connection}
    {
    }

    std::optional<application::StoredResponse> MySqlIdempotencyStore::find(const std::uint64_t userId,
                                                                           const std::string& key) const
    {
        const auto rows = m_connection.query(SELECT_RESPONSE, {userId, key});
        std::optional<application::StoredResponse> response;
        if (!rows.empty())
        {
            const auto& row = rows.front();
            response = application::StoredResponse {SqlColumns::text(row, 0),
                                                    SqlColumns::text(row, 1),
                                                    static_cast<int>(SqlColumns::signedNumber(row, 2)),
                                                    SqlColumns::text(row, 3)};
        }
        return response;
    }

    void MySqlIdempotencyStore::save(const std::uint64_t userId,
                                     const std::string& key,
                                     const application::StoredResponse& response)
    {
        m_connection.execute(
            INSERT_RESPONSE,
            {userId, key, response.method, response.path, std::int64_t {response.status}, response.body});
    }

} // namespace hubflow::shared::adapters
