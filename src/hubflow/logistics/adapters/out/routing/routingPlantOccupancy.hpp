#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_ROUTING_ROUTING_PLANT_OCCUPANCY_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_ROUTING_ROUTING_PLANT_OCCUPANCY_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/domain/services/plantOccupancy.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::logistics::adapters
{

/**
 * @brief Responde qué hay hoy en la operación mirando la lista de pendientes de logística y las rutas y el mapa de
 * ruteo.
 */
class RoutingPlantOccupancy final : public domain::PlantOccupancy
{
public:
    RoutingPlantOccupancy(const application::LogisticsModel&        logistics,
                          const routing::application::RoutingModel& routing);

private:
    bool hasShipmentsInZone(const std::string& zoneCode) const override;
    bool hasShipmentsWithService(const std::string& serviceCode) const override;
    bool hasRoutesInProgressInZone(const std::string& zoneCode) const override;
    bool hasActiveMapNodesInZone(const std::string& zoneCode) const override;

    const application::LogisticsModel&        m_logistics;
    const routing::application::RoutingModel& m_routing;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_ROUTING_ROUTING_PLANT_OCCUPANCY_HPP
