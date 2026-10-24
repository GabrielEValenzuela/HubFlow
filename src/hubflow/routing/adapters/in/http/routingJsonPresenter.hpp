#ifndef HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROUTING_JSON_PRESENTER_HPP
#define HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROUTING_JSON_PRESENTER_HPP

#include "hubflow/platform/http/httpTypes.hpp"
#include "hubflow/routing/application/ports/in/getCourierRouteSheetUseCase.hpp"
#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"

namespace hubflow::routing::adapters
{

    /**
     * @brief Representación externa de hojas de ruta y mapa vial.
     *
     * El plan para el simulador (estado inicial y secuencia) usa los mismos documentos que se exportan a
     * archivos, para que la API y los archivos entreguen exactamente lo mismo.
     */
    class RoutingJsonPresenter final
    {
    public:
        /// @brief Mapa vial: nodos con su zona y coordenadas, y calles con su costo en kilómetros.
        static platform::Json roadMap(const domain::RoadMap& map);

        /// @brief Un punto del mapa con su zona, coordenadas y habilitación.
        static platform::Json node(const domain::RoadNode& node);

        /// @brief Una búsqueda individual con sus pasos, camino y métricas.
        static platform::Json searchTrace(const domain::SearchTrace& trace, const domain::RoadMap& map);

        /// @brief Datos principales de la hoja de ruta, sin paradas.
        static platform::Json routeSummary(const domain::DeliveryRoute& route);

        /// @brief Hoja de ruta con sus paradas y la secuencia completa de búsqueda.
        static platform::Json route(const domain::DeliveryRoute& route, const domain::RoadMap& map);

        /// @brief Hoja de ruta como la ve el repartidor: cada parada suma destinatario, servicio, peso y zona.
        static platform::Json courierSheet(const application::CourierRouteSheet& sheet, const domain::RoadMap& map);

        /// @brief Problema que resolvió el agente, listo para descargar.
        static platform::Json initialState(const domain::DeliveryRoute& route, const domain::RoadMap& map);

        /// @brief Secuencia de búsqueda de la ruta, lista para descargar.
        static platform::Json sequence(const domain::DeliveryRoute& route, const domain::RoadMap& map);
    };

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROUTING_JSON_PRESENTER_HPP
