#include "infrastructure/persistence/mysql/mySqlUserRepository.hpp"

#include "application/shared/vocabulary.hpp"
#include "infrastructure/persistence/mysql/sqlConversions.hpp"

namespace hubflow::infrastructure
{
namespace
{
using application::Vocabulary;
using domain::identity::UserState;

constexpr const char* USER_COLUMNS {"SELECT id, email, name, role, password_hash, active FROM users"};

constexpr const char* NEXT_USER_ID {"SELECT COALESCE(MAX(id), 0) + 1 FROM users"};

constexpr const char* INSERT_USER {R"SQL(
    INSERT INTO users (id, email, name, role, password_hash, active) VALUES (?, ?, ?, ?, ?, ?)
)SQL"};

constexpr const char* UPDATE_USER {R"SQL(
    UPDATE users SET name = ?, role = ?, password_hash = ?, active = ? WHERE id = ?
)SQL"};

constexpr const char* COUNT_USERS {"SELECT COUNT(*) FROM users"};

constexpr const char* INSERT_AUDIT {R"SQL(
    INSERT INTO audit_events (occurred_at, actor_id, actor_name, type, subject, detail) VALUES (?, ?, ?, ?, ?, ?)
)SQL"};

constexpr const char* SELECT_AUDIT {R"SQL(
    SELECT id, occurred_at, actor_id, actor_name, type, subject, detail
    FROM audit_events ORDER BY id DESC LIMIT ? OFFSET ?
)SQL"};

constexpr const char* COUNT_AUDIT {"SELECT COUNT(*) FROM audit_events"};

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

MySqlUserRepository::MySqlUserRepository(MySqlConnection& connection)
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
    m_connection.execute(
        UPDATE_USER,
        {state.name, Vocabulary::name(state.role), state.passwordHash, std::int64_t {state.active ? 1 : 0}, state.id});
}

std::optional<UserState> MySqlUserRepository::byId(const std::uint64_t id) const
{
    return firstUser(m_connection.query(std::string {USER_COLUMNS} + " WHERE id = ?", {id}));
}

std::optional<UserState> MySqlUserRepository::byEmail(const std::string& email) const
{
    return firstUser(m_connection.query(std::string {USER_COLUMNS} + " WHERE email = ?", {email}));
}

void MySqlUserRepository::forEach(domain::identity::UserVisitor& visitor) const
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

MySqlAuditTrail::MySqlAuditTrail(MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlAuditTrail::append(const application::AuditEntry& entry)
{
    m_connection.execute(INSERT_AUDIT,
                         {std::int64_t {entry.occurredAt},
                          entry.actorId,
                          entry.actorName,
                          entry.type,
                          entry.subject,
                          entry.detail});
}

void MySqlAuditTrail::forEachRecent(const std::size_t               limit,
                                    const std::size_t               offset,
                                    application::AuditEntryVisitor& visitor) const
{
    for (const auto& row : m_connection.query(SELECT_AUDIT, {std::uint64_t {limit}, std::uint64_t {offset}}))
    {
        visitor.visit(application::AuditEntry {SqlColumns::unsignedNumber(row, 0),
                                               SqlColumns::signedNumber(row, 1),
                                               SqlColumns::unsignedNumber(row, 2),
                                               SqlColumns::text(row, 3),
                                               SqlColumns::text(row, 4),
                                               SqlColumns::text(row, 5),
                                               SqlColumns::text(row, 6)});
    }
}

std::size_t MySqlAuditTrail::count() const
{
    return SqlColumns::unsignedNumber(m_connection.query(COUNT_AUDIT).front(), 0);
}

} // namespace hubflow::infrastructure
