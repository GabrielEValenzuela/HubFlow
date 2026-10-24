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
     * La igualdad se deduce de operator<: dos elementos son iguales cuando ninguno es menor que el otro.
     *
     * | Operación                | Mejor | Peor     |
     * |--------------------------|-------|----------|
     * | insert / remove          | O(1)  | O(n)     |
     * | contains                 | O(1)  | O(n)     |
     * | unite / intersect / diff | O(n+m)| O(n+m)   |
     *
     * @tparam TElement tipo con operator<.
     */
    template<typename TElement>
    class Set final
    {
    public:
        /// @brief Nodo de la lista ordenada.
        struct Node
        {
            TElement value {};
            Node* pNext {nullptr};
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
            other.m_size = 0;
        }

        Set& operator=(Set&& other) noexcept
        {
            if (this != &other)
            {
                clear();
                m_pHead = other.m_pHead;
                m_size = other.m_size;
                other.m_pHead = nullptr;
                other.m_size = 0;
            }
            return *this;
        }

        /// @return false si el elemento ya pertenecía (no se duplica).
        bool insert(const TElement& value)
        {
            Node** ppLink = lowerBound(value);
            const bool absent {*ppLink == nullptr || value < (*ppLink)->value};
            if (absent)
            {
                *ppLink = new Node {value, *ppLink};
                ++m_size;
            }
            return absent;
        }

        /// @return false si el elemento no pertenecía.
        bool remove(const TElement& value)
        {
            Node** ppLink = lowerBound(value);
            const bool present {*ppLink != nullptr && !(value < (*ppLink)->value)};
            if (present)
            {
                Node* pDoomed = *ppLink;
                *ppLink = pDoomed->pNext;
                delete pDoomed;
                --m_size;
            }
            return present;
        }

        bool contains(const TElement& value) const
        {
            const Node* pCurrent = m_pHead;
            while (pCurrent != nullptr && pCurrent->value < value)
            {
                pCurrent = pCurrent->pNext;
            }
            return pCurrent != nullptr && !(value < pCurrent->value);
        }

        /// @brief Unión de este conjunto con @p other.
        Set unite(const Set& other) const
        {
            Set merged;
            Node** ppTail = &merged.m_pHead;
            const Node* pLeftNode = m_pHead;
            const Node* pRightNode = other.m_pHead;
            while (pLeftNode != nullptr || pRightNode != nullptr)
            {
                if (pRightNode == nullptr || (pLeftNode != nullptr && pLeftNode->value < pRightNode->value))
                {
                    merged.append(ppTail, pLeftNode->value);
                    pLeftNode = pLeftNode->pNext;
                }
                else if (pLeftNode == nullptr || pRightNode->value < pLeftNode->value)
                {
                    merged.append(ppTail, pRightNode->value);
                    pRightNode = pRightNode->pNext;
                }
                else
                {
                    merged.append(ppTail, pLeftNode->value);
                    pLeftNode = pLeftNode->pNext;
                    pRightNode = pRightNode->pNext;
                }
            }
            return merged;
        }

        /// @brief Intersección de este conjunto con @p other.
        Set intersect(const Set& other) const
        {
            Set merged;
            Node** ppTail = &merged.m_pHead;
            const Node* pLeftNode = m_pHead;
            const Node* pRightNode = other.m_pHead;
            while (pLeftNode != nullptr && pRightNode != nullptr)
            {
                if (pLeftNode->value < pRightNode->value)
                {
                    pLeftNode = pLeftNode->pNext;
                }
                else if (pRightNode->value < pLeftNode->value)
                {
                    pRightNode = pRightNode->pNext;
                }
                else
                {
                    merged.append(ppTail, pLeftNode->value);
                    pLeftNode = pLeftNode->pNext;
                    pRightNode = pRightNode->pNext;
                }
            }
            return merged;
        }

        /// @brief Elementos de este conjunto que no están en @p other.
        Set difference(const Set& other) const
        {
            Set merged;
            Node** ppTail = &merged.m_pHead;
            const Node* pLeftNode = m_pHead;
            const Node* pRightNode = other.m_pHead;
            while (pLeftNode != nullptr)
            {
                if (pRightNode == nullptr || pLeftNode->value < pRightNode->value)
                {
                    merged.append(ppTail, pLeftNode->value);
                    pLeftNode = pLeftNode->pNext;
                }
                else if (pRightNode->value < pLeftNode->value)
                {
                    pRightNode = pRightNode->pNext;
                }
                else
                {
                    pLeftNode = pLeftNode->pNext;
                    pRightNode = pRightNode->pNext;
                }
            }
            return merged;
        }

        /// @brief Visita los elementos en orden ascendente.
        template<typename TVisitor>
        void forEach(TVisitor visit) const
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

        bool isEmpty() const noexcept
        {
            return m_pHead == nullptr;
        }

    private:
        Node** lowerBound(const TElement& value)
        {
            Node** ppLink = &m_pHead;
            while (*ppLink != nullptr && (*ppLink)->value < value)
            {
                ppLink = &(*ppLink)->pNext;
            }
            return ppLink;
        }

        void append(Node**& ppTail, const TElement& value)
        {
            *ppTail = new Node {value, nullptr};
            ppTail = &(*ppTail)->pNext;
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

        Node* m_pHead {nullptr};
        std::size_t m_size {0};
    };

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_SET_HPP
