#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_NODE_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_NODE_HPP

#include <cstddef>
#include <cstdint>

namespace hubflow::routing::domain
{

    /// @brief Nodo del árbol de búsqueda: estado, padre (para reconstruir el camino) y costos.
    struct SearchNode
    {
        std::size_t vertex;
        SearchNode* pParent;
        std::size_t depth;
        double cost;
        double priority;
        std::uint64_t order;
    };

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_NODE_HPP
