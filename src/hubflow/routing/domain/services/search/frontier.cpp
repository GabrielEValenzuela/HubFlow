#include "hubflow/routing/domain/services/search/frontier.hpp"

namespace hubflow::routing::domain
{

    ds::DynamicArray<std::size_t> Frontier::snapshot(const ds::DynamicArray<bool>& explored) const
    {
        ds::DynamicArray<const SearchNode*> nodes;
        collect(nodes);
        ds::DynamicArray<bool> seen {explored.size(), false};
        ds::DynamicArray<std::size_t> vertices;
        for (std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
        {
            const std::size_t vertex {nodes[nodeIndex]->vertex};
            if (!explored[vertex] && !seen[vertex])
            {
                seen[vertex] = true;
                vertices.pushBack(vertex);
            }
        }
        return vertices;
    }

    void FifoFrontier::push(SearchNode* pNode)
    {
        m_queue.enqueue(pNode);
    }

    SearchNode* FifoFrontier::pop()
    {
        return m_queue.dequeue();
    }

    bool FifoFrontier::isEmpty() const noexcept
    {
        return m_queue.isEmpty();
    }

    void FifoFrontier::collect(ds::DynamicArray<const SearchNode*>& out) const
    {
        m_queue.forEach([&out](const SearchNode* pNode) { out.pushBack(pNode); });
    }

    void LifoFrontier::push(SearchNode* pNode)
    {
        m_stack.push(pNode);
    }

    SearchNode* LifoFrontier::pop()
    {
        return m_stack.pop();
    }

    bool LifoFrontier::isEmpty() const noexcept
    {
        return m_stack.isEmpty();
    }

    void LifoFrontier::collect(ds::DynamicArray<const SearchNode*>& out) const
    {
        m_stack.forEach([&out](const SearchNode* pNode) { out.pushBack(pNode); });
    }

    bool ByPriority::operator()(const SearchNode* pA, const SearchNode* pB) const noexcept
    {
        return pA->priority < pB->priority || (pA->priority == pB->priority && pA->order < pB->order);
    }

    void PriorityFrontier::push(SearchNode* pNode)
    {
        m_heap.push(pNode);
    }

    SearchNode* PriorityFrontier::pop()
    {
        return m_heap.pop();
    }

    bool PriorityFrontier::isEmpty() const noexcept
    {
        return m_heap.isEmpty();
    }

    void PriorityFrontier::collect(ds::DynamicArray<const SearchNode*>& out) const
    {
        m_heap.forEach([&out](const SearchNode* pNode) { out.pushBack(pNode); });
    }

} // namespace hubflow::routing::domain
