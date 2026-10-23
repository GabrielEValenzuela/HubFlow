#ifndef HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_ROAD_NODE_HPP
#define HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_ROAD_NODE_HPP

#include "hubflow/logistics/domain/valueobjects/zone.hpp"

#include <string>

namespace hubflow::routing::domain
{

/// @brief Intersección o punto de entrega del mapa. Coordenadas en kilómetros.
struct RoadNode
{
    std::string             id;
    std::string             label;
    bool                    isHub {false};
    logistics::domain::Zone zone {logistics::domain::Zone::Centro};
    double                  x {0.0};
    double                  y {0.0};
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_ROAD_NODE_HPP
