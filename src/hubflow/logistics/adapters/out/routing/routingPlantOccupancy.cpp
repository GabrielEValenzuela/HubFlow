#include "hubflow/logistics/adapters/out/routing/routingPlantOccupancy.hpp"

namespace hubflow::logistics::adapters
{

RoutingPlantOccupancy::RoutingPlantOccupancy(const application::LogisticsModel&        logistics,
                                             const routing::application::RoutingModel& routing)
    : m_logistics {logistics}
    , m_routing {routing}
{
}

bool RoutingPlantOccupancy::hasShipmentsInZone(const std::string& zoneCode) const
{
    bool found {false};
    m_logistics.pending.forEach(
        [&found, &zoneCode](const domain::Shipment& shipment)
        {
            found = found || shipment.zone() == zoneCode;
        });
    return found;
}

bool RoutingPlantOccupancy::hasShipmentsWithService(const std::string& serviceCode) const
{
    bool found {false};
    m_logistics.pending.forEach(
        [&found, &serviceCode](const domain::Shipment& shipment)
        {
            found = found || shipment.service() == serviceCode;
        });
    return found;
}

bool RoutingPlantOccupancy::hasRoutesInProgressInZone(const std::string& zoneCode) const
{
    bool found {false};
    m_routing.routes.forEach(
        [this, &found, &zoneCode](const routing::domain::DeliveryRoute& route)
        {
            for (std::size_t i = 0; i < route.stops().size() && !found; ++i)
            {
                const auto& stop = route.stops()[i];
                found            = stop.outcome == routing::domain::StopOutcome::Pending &&
                        m_routing.map.node(m_routing.map.require(stop.nodeId)).zone == zoneCode;
            }
        });
    return found;
}

bool RoutingPlantOccupancy::hasActiveMapNodesInZone(const std::string& zoneCode) const
{
    bool found {false};
    for (std::size_t i = 0; i < m_routing.map.nodeCount() && !found; ++i)
    {
        const auto& node = m_routing.map.node(i);
        found            = node.active && node.zone == zoneCode;
    }
    return found;
}

} // namespace hubflow::logistics::adapters
