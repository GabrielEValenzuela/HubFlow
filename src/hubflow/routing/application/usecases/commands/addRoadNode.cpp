#include "hubflow/routing/application/usecases/commands/addRoadNode.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::application
{

    AddRoadNode::AddRoadNode(RoutingModel& routing,
                             const ZoneDirectory& zones,
                             shared::application::EventBus& bus,
                             const shared::application::Clock& clock)
        : m_routing {routing}
        , m_zones {zones}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::RoadNode& AddRoadNode::execute(const shared::application::Principal& principal,
                                                 const AddRoadNodeRequest& request)
    {
        principal.require(identity::domain::Permission::ManageCatalogs);
        if (!m_zones.isActive(request.zone))
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "road_node.zone_unavailable",
                                                  "La zona no existe o está desactivada"};
        }
        const domain::RoadNode node {request.id, request.label, false, request.zone, request.x, request.y, true};
        const std::size_t index {m_routing.map.declareNode(node, request.links, principal.contextAt(m_clock))};
        m_routing.map.publishEvents(m_bus);
        return m_routing.map.node(index);
    }

} // namespace hubflow::routing::application
