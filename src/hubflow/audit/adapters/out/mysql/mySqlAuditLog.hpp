#ifndef HUBFLOW_AUDIT_ADAPTERS_OUT_MYSQL_MY_SQL_AUDIT_LOG_HPP
#define HUBFLOW_AUDIT_ADAPTERS_OUT_MYSQL_MY_SQL_AUDIT_LOG_HPP

#include "hubflow/audit/application/ports/out/auditLog.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"

namespace hubflow::audit::adapters
{

/// @brief Auditoría de acciones guardada en la tabla @c audit_events.
class MySqlAuditLog final : public application::AuditLog
{
public:
    explicit MySqlAuditLog(platform::MySqlConnection& connection);

private:
    void append(const application::AuditEntry& entry) override;
    void forEachRecent(std::size_t limit, std::size_t offset, application::AuditEntryVisitor& visitor) const override;
    std::size_t count() const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::audit::adapters

#endif // HUBFLOW_AUDIT_ADAPTERS_OUT_MYSQL_MY_SQL_AUDIT_LOG_HPP
