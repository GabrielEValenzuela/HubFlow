#include "hubflow/logistics/application/usecases/commands/createServiceLevel.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

CreateServiceLevel::CreateServiceLevel(LogisticsModel&                   model,
                                       shared::application::EventBus&    bus,
                                       const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::ServiceLevel& CreateServiceLevel::execute(const shared::application::Principal& principal,
                                                        const CreateServiceLevelRequest&      request)
{
    principal.require(identity::domain::Permission::ManageCatalogs);
    domain::ServiceLevel& entry = m_model.services.adopt(
        domain::ServiceLevel::open(request.code, request.name, request.priority, principal.contextAt(m_clock)));
    entry.publishEvents(m_bus);
    return entry;
}

} // namespace hubflow::logistics::application
