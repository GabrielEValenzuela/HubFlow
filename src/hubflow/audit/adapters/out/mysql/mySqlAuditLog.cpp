#include "hubflow/audit/adapters/out/mysql/mySqlAuditLog.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"

namespace hubflow::audit::adapters
{
    namespace
    {
        using platform::SqlColumns;

        constexpr const char* INSERT_AUDIT {R"SQL(
    INSERT INTO audit_events (occurred_at, actor_id, actor_name, type, subject, detail) VALUES (?, ?, ?, ?, ?, ?)
)SQL"};

        constexpr const char* SELECT_AUDIT {R"SQL(
    SELECT id, occurred_at, actor_id, actor_name, type, subject, detail
    FROM audit_events ORDER BY id DESC LIMIT ? OFFSET ?
)SQL"};

        constexpr const char* COUNT_AUDIT {"SELECT COUNT(*) FROM audit_events"};
    } // namespace

    MySqlAuditLog::MySqlAuditLog(platform::MySqlConnection& connection)
        : m_connection {connection}
    {
    }

    void MySqlAuditLog::append(const application::AuditEntry& entry)
    {
        m_connection.execute(
            INSERT_AUDIT,
            {std::int64_t {entry.occurredAt}, entry.actorId, entry.actorName, entry.type, entry.subject, entry.detail});
    }

    void MySqlAuditLog::forEachRecent(const std::size_t limit,
                                      const std::size_t offset,
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

    std::size_t MySqlAuditLog::count() const
    {
        return SqlColumns::unsignedNumber(m_connection.query(COUNT_AUDIT).front(), 0);
    }

} // namespace hubflow::audit::adapters
