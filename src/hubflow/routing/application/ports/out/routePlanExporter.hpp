#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROUTE_PLAN_EXPORTER_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROUTE_PLAN_EXPORTER_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"

namespace hubflow::routing::application
{

/**
 * @brief Entrega el plan de una hoja de ruta a quien lo necesite fuera del sistema.
 *
 * El plan describe el problema que resolvió el agente (mapa, origen y paradas) y la secuencia de
 * búsqueda de cada tramo, de modo que un simulador externo pueda reproducir el recorrido.
 */
class RoutePlanExporter
{
public:
    virtual ~RoutePlanExporter() = default;

    /// @brief Publica el plan de @p route calculado sobre @p map.
    virtual void exportPlan(const domain::RoadMap& map, const domain::DeliveryRoute& route) = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROUTE_PLAN_EXPORTER_HPP
