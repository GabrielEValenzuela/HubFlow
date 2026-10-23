#include "hubflow/logistics/adapters/out/routing/roadMapDeliveryPoints.hpp"

namespace hubflow::logistics::adapters
{

RoadMapDeliveryPoints::RoadMapDeliveryPoints(const routing::domain::RoadMap& map)
    : m_map {map}
{
}

bool RoadMapDeliveryPoints::isServiceable(const std::string& nodeId, const logistics::domain::Zone zone) const
{
    const auto index = m_map.indexOf(nodeId);
    return index.has_value() && !m_map.node(*index).isHub && m_map.node(*index).zone == zone;
}

} // namespace hubflow::logistics::adapters
