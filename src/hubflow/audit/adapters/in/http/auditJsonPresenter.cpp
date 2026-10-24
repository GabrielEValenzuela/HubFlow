#include "hubflow/audit/adapters/in/http/auditJsonPresenter.hpp"

namespace hubflow::audit::adapters
{

    platform::Json AuditJsonPresenter::entry(const application::AuditEntry& entry)
    {
        return platform::Json {{"id", entry.id},
                               {"occurredAt", entry.occurredAt},
                               {"actor", platform::Json {{"id", entry.actorId}, {"name", entry.actorName}}},
                               {"type", entry.type},
                               {"subject", entry.subject},
                               {"detail", entry.detail}};
    }

} // namespace hubflow::audit::adapters
