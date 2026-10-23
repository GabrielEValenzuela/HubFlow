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
 * @tparam T tipo almacenado.
 * @tparam Less comparador: devuelve true si el primer argumento tiene más prioridad.
 */
template <typename T, typename Less>
class MinHeap final
{
public:
    explicit MinHeap(Less less = Less {})
        : m_less {less}
    {
    }

    void push(const T& value)
    {
        m_items.pushBack(value);
        siftUp(m_items.size() - 1);
    }

    /// @throws std::underflow_error si está vacío.
    T pop()
    {
        const T best {top()};
        m_items[0] = m_items[m_items.size() - 1];
        m_items.popBack();
        if (!m_items.empty())
        {
            siftDown(0);
        }
        return best;
    }

    /// @throws std::underflow_error si está vacío.
    const T& top() const
    {
        if (m_items.empty())
        {
            throw std::underflow_error("MinHeap vacío");
        }
        return m_items[0];
    }

    /// @brief Visita el arreglo interno en orden de heap (no ordenado).
    template <typename Visitor>
    void forEach(Visitor visit) const
    {
        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            visit(m_items[i]);
        }
    }

    bool isEmpty() const noexcept
    {
        return m_items.empty();
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
            settled = !m_less(m_items[index], m_items[parent]);
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
            if (left < count && m_less(m_items[left], m_items[best]))
            {
                best = left;
            }
            if (right < count && m_less(m_items[right], m_items[best]))
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

    void swapAt(const std::size_t a, const std::size_t b)
    {
        const T temp {m_items[a]};
        m_items[a] = m_items[b];
        m_items[b] = temp;
    }

    DynamicArray<T> m_items;
    Less            m_less;
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_MIN_HEAP_HPP
