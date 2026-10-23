#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_NODE_POOL_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_NODE_POOL_HPP

#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/routing/domain/services/search/searchNode.hpp"

namespace hubflow::routing::domain
{

/// @brief Dueño de todos los SearchNode creados en una búsqueda; los libera al destruirse.
class SearchNodePool final
{
public:
    SearchNodePool() = default;

    ~SearchNodePool();

    SearchNodePool(const SearchNodePool&)            = delete;
    SearchNodePool& operator=(const SearchNodePool&) = delete;

    /// @brief Crea un nodo hijo de @p pParent (nullptr para la raíz).
    SearchNode* create(std::size_t vertex, SearchNode* pParent, double cost, double priority);

private:
    ds::SinglyLinkedList<SearchNode*> m_nodes;
    std::uint64_t                     m_created {0};
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_NODE_POOL_HPP
