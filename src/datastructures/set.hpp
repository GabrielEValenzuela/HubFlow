#ifndef HUBFLOW_DATASTRUCTURES_SET_HPP
#define HUBFLOW_DATASTRUCTURES_SET_HPP

#include <cstddef>

namespace hubflow::ds
{

/**
 * @brief Conjunto matemático sobre una lista simplemente enlazada ORDENADA y sin repetidos.
 *
 * Mantener el orden permite resolver unión, intersección y diferencia con un único recorrido
 * simultáneo de ambas listas (merge) en O(n + m), en lugar de O(n · m).
 * La igualdad se deduce de operator<: a == b  <=>  !(a < b) && !(b < a).
 *
 * | Operación                | Mejor | Peor     |
 * |--------------------------|-------|----------|
 * | insert / remove          | O(1)  | O(n)     |
 * | contains                 | O(1)  | O(n)     |
 * | unite / intersect / diff | O(n+m)| O(n+m)   |
 *
 * @tparam T tipo con operator<.
 */
template <typename T>
class Set final
{
public:
    /// @brief Nodo de la lista ordenada.
    struct Node
    {
        T     value;
        Node* pNext;
    };

    Set() = default;

    ~Set()
    {
        clear();
    }

    /// @brief Copia profunda de los nodos.
    Set(const Set& other)
    {
        copyFrom(other);
    }

    Set& operator=(const Set& other)
    {
        if (this != &other)
        {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    Set(Set&& other) noexcept
        : m_pHead {other.m_pHead}
        , m_size {other.m_size}
    {
        other.m_pHead = nullptr;
        other.m_size  = 0;
    }

    Set& operator=(Set&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            m_pHead       = other.m_pHead;
            m_size        = other.m_size;
            other.m_pHead = nullptr;
            other.m_size  = 0;
        }
        return *this;
    }

    /// @return false si el elemento ya pertenecía (no se duplica).
    bool insert(const T& value)
    {
        Node**     ppLink = lowerBound(value);
        const bool absent {*ppLink == nullptr || value < (*ppLink)->value};
        if (absent)
        {
            *ppLink = new Node {value, *ppLink};
            ++m_size;
        }
        return absent;
    }

    /// @return false si el elemento no pertenecía.
    bool remove(const T& value)
    {
        Node**     ppLink = lowerBound(value);
        const bool present {*ppLink != nullptr && !(value < (*ppLink)->value)};
        if (present)
        {
            Node* pDoomed = *ppLink;
            *ppLink       = pDoomed->pNext;
            delete pDoomed;
            --m_size;
        }
        return present;
    }

    bool contains(const T& value) const
    {
        const Node* pCurrent = m_pHead;
        while (pCurrent != nullptr && pCurrent->value < value)
        {
            pCurrent = pCurrent->pNext;
        }
        return pCurrent != nullptr && !(value < pCurrent->value);
    }

    /// @brief A ∪ B.
    Set unite(const Set& other) const
    {
        Set         result;
        Node**      ppTail = &result.m_pHead;
        const Node* pA     = m_pHead;
        const Node* pB     = other.m_pHead;
        while (pA != nullptr || pB != nullptr)
        {
            if (pB == nullptr || (pA != nullptr && pA->value < pB->value))
            {
                result.append(ppTail, pA->value);
                pA = pA->pNext;
            }
            else if (pA == nullptr || pB->value < pA->value)
            {
                result.append(ppTail, pB->value);
                pB = pB->pNext;
            }
            else
            {
                result.append(ppTail, pA->value);
                pA = pA->pNext;
                pB = pB->pNext;
            }
        }
        return result;
    }

    /// @brief A ∩ B.
    Set intersect(const Set& other) const
    {
        Set         result;
        Node**      ppTail = &result.m_pHead;
        const Node* pA     = m_pHead;
        const Node* pB     = other.m_pHead;
        while (pA != nullptr && pB != nullptr)
        {
            if (pA->value < pB->value)
            {
                pA = pA->pNext;
            }
            else if (pB->value < pA->value)
            {
                pB = pB->pNext;
            }
            else
            {
                result.append(ppTail, pA->value);
                pA = pA->pNext;
                pB = pB->pNext;
            }
        }
        return result;
    }

    /// @brief A − B.
    Set difference(const Set& other) const
    {
        Set         result;
        Node**      ppTail = &result.m_pHead;
        const Node* pA     = m_pHead;
        const Node* pB     = other.m_pHead;
        while (pA != nullptr)
        {
            if (pB == nullptr || pA->value < pB->value)
            {
                result.append(ppTail, pA->value);
                pA = pA->pNext;
            }
            else if (pB->value < pA->value)
            {
                pB = pB->pNext;
            }
            else
            {
                pA = pA->pNext;
                pB = pB->pNext;
            }
        }
        return result;
    }

    /// @brief Visita los elementos en orden ascendente.
    template <typename Visitor>
    void forEach(Visitor visit) const
    {
        for (const Node* pCurrent = m_pHead; pCurrent != nullptr; pCurrent = pCurrent->pNext)
        {
            visit(pCurrent->value);
        }
    }

    void clear() noexcept
    {
        while (m_pHead != nullptr)
        {
            Node* pNext = m_pHead->pNext;
            delete m_pHead;
            m_pHead = pNext;
        }
        m_size = 0;
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
    Node** lowerBound(const T& value)
    {
        Node** ppLink = &m_pHead;
        while (*ppLink != nullptr && (*ppLink)->value < value)
        {
            ppLink = &(*ppLink)->pNext;
        }
        return ppLink;
    }

    void append(Node**& ppTail, const T& value)
    {
        *ppTail = new Node {value, nullptr};
        ppTail  = &(*ppTail)->pNext;
        ++m_size;
    }

    void copyFrom(const Set& other)
    {
        Node** ppTail = &m_pHead;
        for (const Node* pCurrent = other.m_pHead; pCurrent != nullptr; pCurrent = pCurrent->pNext)
        {
            append(ppTail, pCurrent->value);
        }
    }

    Node*       m_pHead {nullptr};
    std::size_t m_size {0};
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_SET_HPP
