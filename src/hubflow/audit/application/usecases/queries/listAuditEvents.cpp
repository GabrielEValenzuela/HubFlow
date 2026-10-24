#include "hubflow/audit/application/usecases/queries/listAuditEvents.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::audit::application
{

    ListAuditEvents::ListAuditEvents(const AuditLog& log)
        : m_log {log}
    {
    }

    void ListAuditEvents::execute(const shared::application::Principal& principal,
                                  std::size_t limit,
                                  std::size_t offset,
                                  AuditEntryVisitor& visitor) const
    {
        principal.require(identity::domain::Permission::ViewAudit);
        m_log.forEachRecent(limit, offset, visitor);
    }

} // namespace hubflow::audit::application
