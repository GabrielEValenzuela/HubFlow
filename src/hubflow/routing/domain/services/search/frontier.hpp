#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_FRONTIER_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_FRONTIER_HPP

#include "datastructures/dynamicArray.hpp"
#include "datastructures/minHeap.hpp"
#include "datastructures/queue.hpp"
#include "datastructures/stack.hpp"
#include "hubflow/routing/domain/services/search/searchNode.hpp"

namespace hubflow::routing::domain
{

    /**
     * @brief Frontera de la búsqueda: define el orden en que el agente expande los nodos.
     *
     * Guarda punteros NO propietarios a SearchNode (los nodos pertenecen al pool de la búsqueda).
     */
    class Frontier
    {
    public:
        virtual ~Frontier() = default;

        virtual void push(SearchNode* pNode) = 0;
        virtual SearchNode* pop() = 0;
        virtual bool isEmpty() const noexcept = 0;

        /**
         * @brief Vértices presentes en la frontera, sin repetidos y omitiendo los ya explorados
         * (entradas obsoletas que puede dejar A*).
         */
        ds::DynamicArray<std::size_t> snapshot(const ds::DynamicArray<bool>& explored) const;

    protected:
        /// @brief Agrega a @p out los nodos de la frontera en su orden interno.
        virtual void collect(ds::DynamicArray<const SearchNode*>& out) const = 0;
    };

    /// @brief Frontera FIFO sobre la Queue propia (BFS).
    class FifoFrontier final : public Frontier
    {
    private:
        void push(SearchNode* pNode) override;
        SearchNode* pop() override;
        bool isEmpty() const noexcept override;
        void collect(ds::DynamicArray<const SearchNode*>& out) const override;

        ds::Queue<SearchNode*> m_queue;
    };

    /// @brief Frontera LIFO sobre la Stack propia (DFS).
    class LifoFrontier final : public Frontier
    {
    private:
        void push(SearchNode* pNode) override;
        SearchNode* pop() override;
        bool isEmpty() const noexcept override;
        void collect(ds::DynamicArray<const SearchNode*>& out) const override;

        ds::Stack<SearchNode*> m_stack;
    };

    /// @brief Orden de la cola de prioridad: menor prioridad primero; empate por orden de creación.
    struct ByPriority
    {
        bool operator()(const SearchNode* pA, const SearchNode* pB) const noexcept;
    };

    /// @brief Frontera de prioridad sobre el MinHeap propio (Greedy y A*).
    class PriorityFrontier final : public Frontier
    {
    private:
        void push(SearchNode* pNode) override;
        SearchNode* pop() override;
        bool isEmpty() const noexcept override;
        void collect(ds::DynamicArray<const SearchNode*>& out) const override;

        ds::MinHeap<SearchNode*, ByPriority> m_heap;
    };

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_FRONTIER_HPP
