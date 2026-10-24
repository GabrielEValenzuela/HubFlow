#include "hubflow/audit/application/usecases/queries/countAuditEvents.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::audit::application
{

    CountAuditEvents::CountAuditEvents(const AuditLog& log)
        : m_log {log}
    {
    }

    std::size_t CountAuditEvents::execute(const shared::application::Principal& principal) const
    {
        principal.require(identity::domain::Permission::ViewAudit);
        return m_log.count();
    }

} // namespace hubflow::audit::application
