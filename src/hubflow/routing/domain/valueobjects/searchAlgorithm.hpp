#ifndef HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_SEARCH_ALGORITHM_HPP
#define HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_SEARCH_ALGORITHM_HPP

namespace hubflow::routing::domain
{

/// @brief Algoritmo de búsqueda que usa el agente PUM.
enum class SearchAlgorithm
{
    BreadthFirst,
    DepthFirst,
    GreedyBestFirst,
    AStar
};

inline constexpr SearchAlgorithm ALL_ALGORITHMS[] {SearchAlgorithm::BreadthFirst,
                                                   SearchAlgorithm::DepthFirst,
                                                   SearchAlgorithm::GreedyBestFirst,
                                                   SearchAlgorithm::AStar};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_SEARCH_ALGORITHM_HPP
