#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_LOGISTICS_PLANT_TRACKED_SHIPMENTS_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_LOGISTICS_PLANT_TRACKED_SHIPMENTS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/tracking/application/ports/out/trackedShipmentDirectory.hpp"

namespace hubflow::tracking::adapters
{

    /**
     * @brief Arma la vista de seguimiento de un paquete consultando el catálogo de envíos de logística y el mapa de
     * ruteo.
     *
     * Traduce los movimientos a hitos del cliente y deja afuera las notas internas y los nombres del personal. Entrega
     * el teléfono y el código postal sólo como datos de comparación para la verificación.
     */
    class PlantTrackedShipments final : public application::TrackedShipmentDirectory
    {
    public:
        PlantTrackedShipments(const logistics::application::LogisticsModel& logistics,
                              const routing::application::RoutingModel& routing);

    private:
        std::optional<application::TrackedShipment> find(const std::string& code) const override;

        std::string neighbourhoodOf(const std::string& nodeId) const;

        const logistics::application::LogisticsModel& m_logistics;
        const routing::application::RoutingModel& m_routing;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_LOGISTICS_PLANT_TRACKED_SHIPMENTS_HPP
