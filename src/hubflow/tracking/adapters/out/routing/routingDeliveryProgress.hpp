#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_ROUTING_ROUTING_DELIVERY_PROGRESS_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_ROUTING_ROUTING_DELIVERY_PROGRESS_HPP

#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/tracking/application/ports/out/deliveryProgressSource.hpp"

namespace hubflow::tracking::adapters
{

/**
 * @brief Calcula cuánto falta para la llegada de un paquete a partir de la hoja de ruta que lo lleva.
 *
 * Toma la última hoja de ruta que incluye al paquete en una parada pendiente. Suma los kilómetros de los tramos que
 * faltan desde donde está el repartidor (la última parada resuelta, o el centro de distribución) hasta esa parada y
 * aplica la política de tiempos de ruteo, la misma que usa la planta al comparar rutas.
 */
class RoutingDeliveryProgress final : public application::DeliveryProgressSource
{
public:
    explicit RoutingDeliveryProgress(const routing::application::RoutingModel& routing);

private:
    std::optional<application::DeliveryProgress> progressOf(const std::string& code) const override;

    const routing::application::RoutingModel& m_routing;
};

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_ROUTING_ROUTING_DELIVERY_PROGRESS_HPP
