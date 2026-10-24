#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_ROUTE_PLAN_JSON_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_ROUTE_PLAN_JSON_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"

#include <nlohmann/json.hpp>

namespace hubflow::routing::adapters
{

    /**
     * @brief Documentos JSON que describen el plan de una hoja de ruta para un simulador externo.
     *
     * Los comparten el exportador a archivos y la API REST, así ambos entregan exactamente lo mismo.
     */
    class RoutePlanJson final
    {
    public:
        /**
         * @brief Problema que resolvió el agente: id de ruta, repartidor, algoritmo, origen (HUB), paradas
         * como objetivos, nodos del mapa y aristas con su costo en km.
         */
        static nlohmann::json initialState(const domain::RoadMap& map, const domain::DeliveryRoute& route);

        /**
         * @brief Secuencia de búsqueda de cada tramo (HUB hacia la primera parada, entre paradas y regreso al
         * HUB) con pasos, camino y métricas, más el camino completo y las métricas totales.
         */
        static nlohmann::json sequence(const domain::RoadMap& map, const domain::DeliveryRoute& route);

        /// @brief Nodos y aristas del mapa vial, tal como aparecen en el estado inicial.
        static nlohmann::json roadMap(const domain::RoadMap& map);

        /// @brief Una búsqueda individual: algoritmo, origen, destino, pasos, camino y métricas.
        static nlohmann::json searchTrace(const domain::RoadMap& map, const domain::SearchTrace& trace);
    };

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_ROUTE_PLAN_JSON_HPP
