#ifndef HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROAD_MAP_CONTROLLER_HPP
#define HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROAD_MAP_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"
#include "hubflow/routing/application/ports/in/addRoadNodeUseCase.hpp"
#include "hubflow/routing/application/ports/in/getRoadMapUseCase.hpp"
#include "hubflow/routing/application/ports/in/previewRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/updateRoadNodeUseCase.hpp"

namespace hubflow::routing::adapters
{

    /**
     * @brief Mapa vial: consulta, vista previa de recorridos y alta de puntos nuevos.
     *
     * - @c GET /road-map: nodos y calles de la ciudad.
     * - @c POST /road-nodes {id, label, zone, x, y, links:[{to, costKm}]}: agrega un punto de entrega a una zona y lo
     *   une a la red (acepta @c Idempotency-Key; repetir el id responde 409). Responde 201 con @c Location.
     * - @c PATCH /road-nodes/:id {zone?, active?}: pasa el punto a otra zona o lo activa o desactiva.
     * - @c GET /route-previews?from=&to=&algorithm=: recorrido que encontraría el agente entre dos puntos, con
     *   todos los pasos de la búsqueda, sin crear ninguna hoja de ruta.
     */
    class RoadMapController final : public platform::Controller
    {
    public:
        RoadMapController(const application::GetRoadMapUseCase& getRoadMap,
                          const application::PreviewRouteUseCase& preview,
                          application::AddRoadNodeUseCase& addNode,
                          application::UpdateRoadNodeUseCase& updateNode);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        const application::GetRoadMapUseCase& m_getRoadMap;
        const application::PreviewRouteUseCase& m_preview;
        application::AddRoadNodeUseCase& m_addNode;
        application::UpdateRoadNodeUseCase& m_updateNode;
    };

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROAD_MAP_CONTROLLER_HPP
