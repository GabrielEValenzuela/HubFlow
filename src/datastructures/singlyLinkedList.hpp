#ifndef HUBFLOW_DATASTRUCTURES_SINGLY_LINKED_LIST_HPP
#define HUBFLOW_DATASTRUCTURES_SINGLY_LINKED_LIST_HPP

#include <cstddef>

namespace hubflow::ds
{

/**
 * @brief Lista simplemente enlazada con punteros a cabeza y cola.
 *
 * Ownership: crea y destruye únicamente sus nodos. Si @p T es un puntero, el objeto apuntado
 * nunca se libera: queda a cargo de su dueño.
 *
 * | Operación     | Complejidad |
 * |---------------|-------------|
 * | pushFront     | O(1)        |
 * | pushBack      | O(1)        |
 * | popFront      | O(1)        |
 * | removeFirstIf | O(n)        |
 * | findFirst     | O(n)        |
 *
 * @tparam T tipo almacenado (copiable).
 */
template <typename T>
class SinglyLinkedList final
{
public:
    /// @brief Nodo: valor y enlace al siguiente.
    struct Node
    {
        T     value;
        Node* pNext;
    };

    SinglyLinkedList() = default;

    ~SinglyLinkedList()
    {
        clear();
    }

    SinglyLinkedList(const SinglyLinkedList&)            = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    /// @brief Transfiere los nodos de @p other, que queda vacía.
    SinglyLinkedList(SinglyLinkedList&& other) noexcept
        : m_pHead {other.m_pHead}
        , m_pTail {other.m_pTail}
        , m_size {other.m_size}
    {
        other.m_pHead = nullptr;
        other.m_pTail = nullptr;
        other.m_size  = 0;
    }

    /// @brief Libera los nodos propios y toma los de @p other.
    SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            m_pHead       = other.m_pHead;
            m_pTail       = other.m_pTail;
            m_size        = other.m_size;
            other.m_pHead = nullptr;
            other.m_pTail = nullptr;
            other.m_size  = 0;
        }
        return *this;
    }

    /// @brief Inserta al inicio. O(1).
    void pushFront(const T& value)
    {
        m_pHead = new Node {value, m_pHead};
        if (m_pTail == nullptr)
        {
            m_pTail = m_pHead;
        }
        ++m_size;
    }

    /// @brief Inserta al final usando el puntero a la cola. O(1).
    void pushBack(const T& value)
    {
        Node* pNode = new Node {value, nullptr};
        if (m_pTail == nullptr)
        {
            m_pHead = pNode;
        }
        else
        {
            m_pTail->pNext = pNode;
        }
        m_pTail = pNode;
        ++m_size;
    }

    /**
     * @brief Extrae el primer elemento.
     * @param[out] out valor extraído.
     * @return false si la lista estaba vacía.
     */
    bool popFront(T& out)
    {
        const bool hasElements {m_pHead != nullptr};
        if (hasElements)
        {
            Node* pFirst = m_pHead;
            out          = pFirst->value;
            m_pHead      = pFirst->pNext;
            if (m_pHead == nullptr)
            {
                m_pTail = nullptr;
            }
            delete pFirst;
            --m_size;
        }
        return hasElements;
    }

    /**
     * @brief Elimina el primer nodo cuyo valor cumple @p matches.
     * @return true si eliminó un nodo.
     */
    template <typename Predicate>
    bool removeFirstIf(Predicate matches)
    {
        Node* pPrevious = nullptr;
        Node* pCurrent  = m_pHead;
        while (pCurrent != nullptr && !matches(pCurrent->value))
        {
            pPrevious = pCurrent;
            pCurrent  = pCurrent->pNext;
        }
        const bool found {pCurrent != nullptr};
        if (found)
        {
            unlink(pPrevious, pCurrent);
        }
        return found;
    }

    /// @brief Primer valor que cumple @p matches, o nullptr. O(n).
    template <typename Predicate>
    const T* findFirst(Predicate matches) const
    {
        const T* pFound = nullptr;
        for (Node* pCurrent = m_pHead; pCurrent != nullptr && pFound == nullptr; pCurrent = pCurrent->pNext)
        {
            if (matches(pCurrent->value))
            {
                pFound = &pCurrent->value;
            }
        }
        return pFound;
    }

    /// @brief Visita los valores de la cabeza a la cola.
    template <typename Visitor>
    void forEach(Visitor visit) const
    {
        for (Node* pCurrent = m_pHead; pCurrent != nullptr; pCurrent = pCurrent->pNext)
        {
            visit(pCurrent->value);
        }
    }

    /// @brief Libera todos los nodos.
    void clear() noexcept
    {
        while (m_pHead != nullptr)
        {
            Node* pNext = m_pHead->pNext;
            delete m_pHead;
            m_pHead = pNext;
        }
        m_pTail = nullptr;
        m_size  = 0;
    }

    /// @brief Primer nodo (para recorridos externos y recursivos).
    const Node* head() const noexcept
    {
        return m_pHead;
    }

    std::size_t size() const noexcept
    {
        return m_size;
    }

    bool empty() const noexcept
    {
        return m_pHead == nullptr;
    }

private:
    void unlink(Node* pPrevious, Node* pDoomed) noexcept
    {
        if (pPrevious == nullptr)
        {
            m_pHead = pDoomed->pNext;
        }
        else
        {
            pPrevious->pNext = pDoomed->pNext;
        }
        if (pDoomed == m_pTail)
        {
            m_pTail = pPrevious;
        }
        delete pDoomed;
        --m_size;
    }

    Node*       m_pHead {nullptr};
    Node*       m_pTail {nullptr};
    std::size_t m_size {0};
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_SINGLY_LINKED_LIST_HPP
