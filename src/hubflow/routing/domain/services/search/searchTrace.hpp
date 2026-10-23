#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_TRACE_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_TRACE_HPP

#include "datastructures/dynamicArray.hpp"
#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/routing/domain/valueobjects/searchAlgorithm.hpp"

#include <cstddef>

namespace hubflow::routing::domain
{

/// @brief Una expansión del agente: nodo expandido y fotografía de frontera y explorados.
struct TraceStep
{
    std::size_t                   step {0};
    std::size_t                   expanded {0};
    ds::DynamicArray<std::size_t> frontier;
    ds::DynamicArray<std::size_t> explored;
};

/// @brief Métricas de la búsqueda (los mismos nombres que usa el JSON exportado al simulador).
struct SearchMetrics
{
    bool        solutionFound {false};
    std::size_t nodesExplored {0};
    std::size_t statesVisited {0};
    std::size_t nodesInFrontier {0};
    std::size_t maxDepth {0};
    double      costTotal {0.0};
};

/// @brief Traza completa de una búsqueda de @c start a @c goal (equivale a sequence.json).
struct SearchTrace
{
    SearchAlgorithm                 algorithm {SearchAlgorithm::BreadthFirst};
    std::size_t                     start {0};
    std::size_t                     goal {0};
    ds::SinglyLinkedList<TraceStep> steps;
    ds::DynamicArray<std::size_t>   path;
    SearchMetrics                   metrics;
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_TRACE_HPP
