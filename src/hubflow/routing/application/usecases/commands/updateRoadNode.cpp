#include "hubflow/routing/application/usecases/commands/updateRoadNode.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::application
{

    UpdateRoadNode::UpdateRoadNode(RoutingModel& routing,
                                   const ZoneDirectory& zones,
                                   shared::application::EventBus& bus,
                                   const shared::application::Clock& clock)
        : m_routing {routing}
        , m_zones {zones}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::RoadNode& UpdateRoadNode::execute(const shared::application::Principal& principal,
                                                    const std::string& nodeId,
                                                    const UpdateRoadNodeRequest& request)
    {
        principal.require(identity::domain::Permission::ManageCatalogs);
        const std::size_t index {m_routing.map.require(nodeId)};
        const bool becomesActive {request.active.value_or(m_routing.map.node(index).active)};
        const std::string targetZone {request.zone.value_or(m_routing.map.node(index).zone)};
        const bool zoneChanges {request.zone.has_value() && *request.zone != m_routing.map.node(index).zone};
        const bool reactivates {request.active.value_or(false) && !m_routing.map.node(index).active};
        if ((zoneChanges || reactivates) && becomesActive && !m_zones.isActive(targetZone))
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::Validation,
                                                 "road_node.zone_unavailable",
                                                 "La zona no existe o está desactivada"};
        }
        m_routing.map.reviseNode(nodeId, request.zone, request.active, principal.contextAt(m_clock));
        m_routing.map.publishEvents(m_bus);
        return m_routing.map.node(index);
    }

} // namespace hubflow::routing::application
