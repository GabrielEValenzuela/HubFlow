#ifndef HUBFLOW_DATASTRUCTURES_MIN_HEAP_HPP
#define HUBFLOW_DATASTRUCTURES_MIN_HEAP_HPP

#include "datastructures/dynamicArray.hpp"

#include <cstddef>
#include <stdexcept>

namespace hubflow::ds
{

/**
 * @brief Montículo binario de mínimos (cola de prioridad) sobre DynamicArray.
 *
 * push y pop son O(log n); top es O(1). Se usa como frontera de Greedy y A*.
 *
 * @tparam TElement tipo almacenado.
 * @tparam TPriorityOrder criterio de prioridad: devuelve true si el primer argumento tiene más prioridad.
 */
template <typename TElement, typename TPriorityOrder>
class MinHeap final
{
public:
    explicit MinHeap(TPriorityOrder hasHigherPriority = TPriorityOrder {})
        : m_hasHigherPriority {hasHigherPriority}
    {
    }

    void push(const TElement& value)
    {
        m_items.pushBack(value);
        siftUp(m_items.size() - 1);
    }

    /// @throws std::underflow_error si está vacío.
    TElement pop()
    {
        const TElement best {top()};
        m_items[0] = m_items[m_items.size() - 1];
        m_items.popBack();
        if (!m_items.isEmpty())
        {
            siftDown(0);
        }
        return best;
    }

    /// @throws std::underflow_error si está vacío.
    const TElement& top() const
    {
        if (m_items.isEmpty())
        {
            throw std::underflow_error("MinHeap vacío");
        }
        return m_items[0];
    }

    /// @brief Visita el arreglo interno en orden de heap (no ordenado).
    template <typename TVisitor>
    void forEach(TVisitor visit) const
    {
        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            visit(m_items[i]);
        }
    }

    bool isEmpty() const noexcept
    {
        return m_items.isEmpty();
    }

    std::size_t size() const noexcept
    {
        return m_items.size();
    }

private:
    void siftUp(std::size_t index)
    {
        bool settled {false};
        while (index > 0 && !settled)
        {
            const std::size_t parent {(index - 1) / 2};
            settled = !m_hasHigherPriority(m_items[index], m_items[parent]);
            if (!settled)
            {
                swapAt(index, parent);
                index = parent;
            }
        }
    }

    void siftDown(std::size_t index)
    {
        const std::size_t count {m_items.size()};
        bool              settled {false};
        while (!settled)
        {
            const std::size_t left {2 * index + 1};
            const std::size_t right {left + 1};
            std::size_t       best {index};
            if (left < count && m_hasHigherPriority(m_items[left], m_items[best]))
            {
                best = left;
            }
            if (right < count && m_hasHigherPriority(m_items[right], m_items[best]))
            {
                best = right;
            }
            settled = best == index;
            if (!settled)
            {
                swapAt(index, best);
                index = best;
            }
        }
    }

    void swapAt(const std::size_t first, const std::size_t second)
    {
        const TElement displaced {m_items[first]};
        m_items[first]  = m_items[second];
        m_items[second] = displaced;
    }

    DynamicArray<TElement> m_items;
    TPriorityOrder         m_hasHigherPriority;
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_MIN_HEAP_HPP
