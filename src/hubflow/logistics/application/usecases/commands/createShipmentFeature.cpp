#include "hubflow/logistics/application/usecases/commands/createShipmentFeature.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    CreateShipmentFeature::CreateShipmentFeature(LogisticsModel& model,
                                                 shared::application::EventBus& bus,
                                                 const shared::application::Clock& clock)
        : m_model {model}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::ShipmentFeature& CreateShipmentFeature::execute(const shared::application::Principal& principal,
                                                                  const CreateShipmentFeatureRequest& request)
    {
        principal.require(identity::domain::Permission::ManageCatalogs);
        domain::ShipmentFeature& entry = m_model.features.adopt(
            domain::ShipmentFeature::open(request.code, request.name, principal.contextAt(m_clock)));
        entry.publishEvents(m_bus);
        return entry;
    }

} // namespace hubflow::logistics::application
