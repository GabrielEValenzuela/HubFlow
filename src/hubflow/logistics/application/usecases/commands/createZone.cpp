#include "hubflow/logistics/application/usecases/commands/createZone.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

CreateZone::CreateZone(LogisticsModel&                   model,
                       shared::application::EventBus&    bus,
                       const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::Zone& CreateZone::execute(const shared::application::Principal& principal,
                                        const CreateZoneRequest&              request)
{
    principal.require(identity::domain::Permission::ManageCatalogs);
    domain::Zone& entry =
        m_model.zones.adopt(domain::Zone::open(request.code, request.name, principal.contextAt(m_clock)));
    entry.publishEvents(m_bus);
    return entry;
}

} // namespace hubflow::logistics::application
