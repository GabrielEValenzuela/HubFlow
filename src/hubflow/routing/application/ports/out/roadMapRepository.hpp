#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROAD_MAP_REPOSITORY_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROAD_MAP_REPOSITORY_HPP

#include "hubflow/routing/domain/entities/roadMap.hpp"

namespace hubflow::routing::application
{

    /// @brief Fuente y destino del mapa de la ciudad.
    class RoadMapRepository
    {
    public:
        virtual ~RoadMapRepository() = default;

        /// @brief Carga nodos y aristas en @p map (vacío).
        virtual void loadInto(domain::RoadMap& map) const = 0;

        /// @brief Persiste un punto nuevo junto con las calles que lo unen a puntos existentes.
        virtual void addNode(const domain::RoadNode& node, const ds::DynamicArray<domain::RoadLink>& links) = 0;

        /// @brief Actualiza la zona y la habilitación de un punto existente.
        virtual void updateNode(const domain::RoadNode& node) = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROAD_MAP_REPOSITORY_HPP
