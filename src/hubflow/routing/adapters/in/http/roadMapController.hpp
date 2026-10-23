#ifndef HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROAD_MAP_CONTROLLER_HPP
#define HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROAD_MAP_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"
#include "hubflow/routing/application/ports/in/getRoadMapUseCase.hpp"
#include "hubflow/routing/application/ports/in/previewRouteUseCase.hpp"

namespace hubflow::routing::adapters
{

/**
 * @brief Mapa vial y vista previa de recorridos (consultas, sin efectos).
 *
 * - @c GET /road-map: nodos y calles de la ciudad.
 * - @c GET /route-previews?from=&to=&algorithm=: recorrido que encontraría el agente entre dos puntos, con
 *   todos los pasos de la búsqueda, sin crear ninguna hoja de ruta.
 */
class RoadMapController final : public platform::Controller
{
public:
    RoadMapController(const application::GetRoadMapUseCase&   getRoadMap,
                      const application::PreviewRouteUseCase& preview);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    const application::GetRoadMapUseCase&   m_getRoadMap;
    const application::PreviewRouteUseCase& m_preview;
};

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROAD_MAP_CONTROLLER_HPP
