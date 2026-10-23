#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROAD_MAP_REPOSITORY_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROAD_MAP_REPOSITORY_HPP

#include "hubflow/routing/domain/entities/roadMap.hpp"

namespace hubflow::routing::application
{

/// @brief Fuente del mapa de la ciudad.
class RoadMapRepository
{
public:
    virtual ~RoadMapRepository() = default;

    /// @brief Carga nodos y aristas en @p map (vacío).
    virtual void loadInto(domain::RoadMap& map) const = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROAD_MAP_REPOSITORY_HPP
