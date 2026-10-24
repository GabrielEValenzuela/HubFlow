#ifndef HUBFLOW_ROUTING_APPLICATION_ROUTING_MODEL_HPP
#define HUBFLOW_ROUTING_APPLICATION_ROUTING_MODEL_HPP

#include "hubflow/routing/domain/entities/roadMap.hpp"
#include "hubflow/routing/domain/services/routeBook.hpp"

namespace hubflow::routing::application
{

    /// @brief Modelo en memoria del contexto de ruteo: mapa de la ciudad y hojas de ruta.
    struct RoutingModel
    {
        domain::RoadMap map;
        domain::RouteBook routes;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_ROUTING_MODEL_HPP
