#ifndef HUBFLOW_DATASTRUCTURES_QUEUE_HPP
#define HUBFLOW_DATASTRUCTURES_QUEUE_HPP

#include <cstddef>
#include <stdexcept>

namespace hubflow::ds
{

/**
 * @brief Cola FIFO enlazada con punteros a front y rear.
 *
 * Mantener @c m_pRear hace que enqueue sea O(1); sin él, encolar requeriría recorrer toda la
 * cola (O(n)). dequeue y front son O(1).
 * Ownership: libera sólo sus nodos; si @p TElement es puntero nunca ejecuta @c delete sobre él.
 *
 * @tparam TElement tipo almacenado (copiable).
 */
template <typename TElement>
class Queue final
{
public:
    /// @brief Nodo con enlace al siguiente en salir.
    struct Node
    {
        TElement value;
        Node*    pNext;
    };

    Queue() = default;

    ~Queue()
    {
        clear();
    }

    Queue(const Queue&)            = delete;
    Queue& operator=(const Queue&) = delete;

    /// @brief Encola al final. O(1).
    void enqueue(const TElement& value)
    {
        Node* pNode = new Node {value, nullptr};
        if (m_pRear == nullptr)
        {
            m_pFront = pNode;
        }
        else
        {
            m_pRear->pNext = pNode;
        }
        m_pRear = pNode;
        ++m_size;
    }

    /**
     * @brief Desencola el frente. O(1).
     * @throws std::underflow_error si la cola está vacía.
     */
    TElement dequeue()
    {
        requireElements();
        Node*          pOld = m_pFront;
        const TElement value {pOld->value};
        m_pFront = pOld->pNext;
        if (m_pFront == nullptr)
        {
            m_pRear = nullptr;
        }
        delete pOld;
        --m_size;
        return value;
    }

    /**
     * @brief Elemento del frente. O(1).
     * @throws std::underflow_error si la cola está vacía.
     */
    const TElement& front() const
    {
        requireElements();
        return m_pFront->value;
    }

    /**
     * @brief Elemento del final. O(1).
     * @throws std::underflow_error si la cola está vacía.
     */
    const TElement& rear() const
    {
        requireElements();
        return m_pRear->value;
    }

    /// @brief Recorre de front a rear.
    template <typename TVisitor>
    void forEach(TVisitor visit) const
    {
        for (Node* pCurrent = m_pFront; pCurrent != nullptr; pCurrent = pCurrent->pNext)
        {
            visit(pCurrent->value);
        }
    }

    /// @brief Libera todos los nodos.
    void clear() noexcept
    {
        while (m_pFront != nullptr)
        {
            Node* pNext = m_pFront->pNext;
            delete m_pFront;
            m_pFront = pNext;
        }
        m_pRear = nullptr;
        m_size  = 0;
    }

    bool isEmpty() const noexcept
    {
        return m_pFront == nullptr;
    }

    std::size_t size() const noexcept
    {
        return m_size;
    }

private:
    void requireElements() const
    {
        if (m_pFront == nullptr)
        {
            throw std::underflow_error("Queue vacía");
        }
    }

    Node*       m_pFront {nullptr};
    Node*       m_pRear {nullptr};
    std::size_t m_size {0};
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_QUEUE_HPP
