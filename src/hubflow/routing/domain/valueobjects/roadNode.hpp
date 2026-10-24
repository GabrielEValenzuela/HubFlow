#ifndef HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_ROAD_NODE_HPP
#define HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_ROAD_NODE_HPP

#include <string>

namespace hubflow::routing::domain
{

/**
 * @brief Intersección o punto de entrega del mapa. Coordenadas en kilómetros.
 *
 * La zona se indica por su código de catálogo. Un punto inactivo deja de aceptar paquetes nuevos pero la calle sigue
 * existiendo, por lo que los recorridos pueden seguir pasando por él.
 */
struct RoadNode
{
    std::string id;
    std::string label;
    bool        isHub {false};
    std::string zone;
    double      x {0.0};
    double      y {0.0};
    bool        active {true};
};

/// @brief Calle que une un punto nuevo con otro ya existente, con su largo en kilómetros.
struct RoadLink
{
    std::string destinationId;
    double      costKm {0.0};
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_ROAD_NODE_HPP
