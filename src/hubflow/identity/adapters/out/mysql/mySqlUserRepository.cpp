#include "hubflow/identity/adapters/out/mysql/mySqlUserRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::identity::adapters
{
    namespace
    {
        using domain::UserState;
        using platform::SqlColumns;
        using platform::SqlRow;
        using shared::application::Vocabulary;

        constexpr const char* USER_COLUMNS {"SELECT id, email, name, role, password_hash, active FROM users"};

        constexpr const char* NEXT_USER_ID {"SELECT COALESCE(MAX(id), 0) + 1 FROM users"};

        constexpr const char* INSERT_USER {R"SQL(
    INSERT INTO users (id, email, name, role, password_hash, active) VALUES (?, ?, ?, ?, ?, ?)
)SQL"};

        constexpr const char* UPDATE_USER {R"SQL(
    UPDATE users SET name = ?, role = ?, password_hash = ?, active = ? WHERE id = ?
)SQL"};

        constexpr const char* COUNT_USERS {"SELECT COUNT(*) FROM users"};

        UserState toUser(const SqlRow& row)
        {
            return UserState {SqlColumns::unsignedNumber(row, 0),
                              SqlColumns::text(row, 1),
                              SqlColumns::text(row, 2),
                              SqlColumns::term(Vocabulary::role(SqlColumns::text(row, 3)), "role"),
                              SqlColumns::text(row, 4),
                              SqlColumns::flag(row, 5)};
        }

        std::optional<UserState> firstUser(const std::vector<SqlRow>& rows)
        {
            return rows.empty() ? std::nullopt : std::optional<UserState> {toUser(rows.front())};
        }
    } // namespace

    MySqlUserRepository::MySqlUserRepository(platform::MySqlConnection& connection)
        : m_connection {connection}
    {
    }

    std::uint64_t MySqlUserRepository::nextIdentity()
    {
        return SqlColumns::unsignedNumber(m_connection.query(NEXT_USER_ID).front(), 0);
    }

    void MySqlUserRepository::add(const UserState& state)
    {
        m_connection.execute(INSERT_USER,
                             {state.id,
                              state.email,
                              state.name,
                              Vocabulary::name(state.role),
                              state.passwordHash,
                              std::int64_t {state.active ? 1 : 0}});
    }

    void MySqlUserRepository::update(const UserState& state)
    {
        m_connection.execute(UPDATE_USER,
                             {state.name,
                              Vocabulary::name(state.role),
                              state.passwordHash,
                              std::int64_t {state.active ? 1 : 0},
                              state.id});
    }

    std::optional<UserState> MySqlUserRepository::byId(const std::uint64_t id) const
    {
        return firstUser(m_connection.query(std::string {USER_COLUMNS} + " WHERE id = ?", {id}));
    }

    std::optional<UserState> MySqlUserRepository::byEmail(const std::string& email) const
    {
        return firstUser(m_connection.query(std::string {USER_COLUMNS} + " WHERE email = ?", {email}));
    }

    void MySqlUserRepository::forEach(application::UserVisitor& visitor) const
    {
        for (const auto& row : m_connection.query(std::string {USER_COLUMNS} + " ORDER BY id"))
        {
            visitor.visit(toUser(row));
        }
    }

    std::size_t MySqlUserRepository::count() const
    {
        return SqlColumns::unsignedNumber(m_connection.query(COUNT_USERS).front(), 0);
    }

} // namespace hubflow::identity::adapters
