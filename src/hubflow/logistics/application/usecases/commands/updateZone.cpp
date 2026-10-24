#include "hubflow/logistics/application/usecases/commands/updateZone.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    UpdateZone::UpdateZone(LogisticsModel& model,
                           const domain::PlantOccupancy& occupancy,
                           shared::application::EventBus& bus,
                           const shared::application::Clock& clock)
        : m_model {model}
        , m_occupancy {occupancy}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::Zone& UpdateZone::execute(const shared::application::Principal& principal,
                                            const std::string& code,
                                            const UpdateZoneRequest& request)
    {
        principal.require(identity::domain::Permission::ManageCatalogs);
        domain::Zone& entry = m_model.zones.require(code);
        entry.revise(request.name, request.active, m_occupancy, principal.contextAt(m_clock));
        entry.publishEvents(m_bus);
        return entry;
    }

} // namespace hubflow::logistics::application
