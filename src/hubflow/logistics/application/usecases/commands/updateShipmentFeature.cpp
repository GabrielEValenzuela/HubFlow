#include "hubflow/logistics/application/usecases/commands/updateShipmentFeature.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

UpdateShipmentFeature::UpdateShipmentFeature(LogisticsModel&                   model,
                                             shared::application::EventBus&    bus,
                                             const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::ShipmentFeature& UpdateShipmentFeature::execute(const shared::application::Principal& principal,
                                                              const std::string&                    code,
                                                              const UpdateShipmentFeatureRequest&   request)
{
    principal.require(identity::domain::Permission::ManageCatalogs);
    domain::ShipmentFeature& entry = m_model.features.require(code);
    entry.revise(request.name, request.active, principal.contextAt(m_clock));
    entry.publishEvents(m_bus);
    return entry;
}

} // namespace hubflow::logistics::application
