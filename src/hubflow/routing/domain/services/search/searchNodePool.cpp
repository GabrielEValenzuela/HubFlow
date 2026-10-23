#include "hubflow/routing/domain/services/search/searchNodePool.hpp"

namespace hubflow::routing::domain
{

SearchNodePool::~SearchNodePool()
{
    m_nodes.forEach(
        [](SearchNode* pNode)
        {
            delete pNode;
        });
}

SearchNode*
SearchNodePool::create(const std::size_t vertex, SearchNode* pParent, const double cost, const double priority)
{
    const std::size_t depth {pParent == nullptr ? 0 : pParent->depth + 1};
    SearchNode*       pNode = new SearchNode {vertex, pParent, depth, cost, priority, m_created++};
    m_nodes.pushBack(pNode);
    return pNode;
}

} // namespace hubflow::routing::domain
