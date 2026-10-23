#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_ROUTING_ROAD_MAP_DELIVERY_POINTS_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_ROUTING_ROAD_MAP_DELIVERY_POINTS_HPP

#include "hubflow/logistics/domain/services/deliveryPointCatalog.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"

namespace hubflow::logistics::adapters
{

/// @brief Traduce el mapa del contexto de ruteo al puerto DeliveryPointCatalog de logística.
class RoadMapDeliveryPoints final : public logistics::domain::DeliveryPointCatalog
{
public:
    explicit RoadMapDeliveryPoints(const routing::domain::RoadMap& map);

private:
    bool isServiceable(const std::string& nodeId, logistics::domain::Zone zone) const override;

    const routing::domain::RoadMap& m_map;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_ROUTING_ROAD_MAP_DELIVERY_POINTS_HPP
