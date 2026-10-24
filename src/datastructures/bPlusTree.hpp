#ifndef HUBFLOW_DATASTRUCTURES_B_PLUS_TREE_HPP
#define HUBFLOW_DATASTRUCTURES_B_PLUS_TREE_HPP

#include <cstddef>

namespace hubflow::ds
{

    /**
     * @brief Árbol B+ de orden @p ORDER con hojas enlazadas y valores NO propietarios.
     *
     * - Nodos internos: hasta ORDER-1 claves separadoras y ORDER hijos. Una clave mayor o igual
     *   al separador desciende por la derecha.
     * - Hojas: claves y punteros a @c TValue, enlazadas por @c pNext para recorridos secuenciales.
     * - Todas las hojas quedan a la misma profundidad porque el árbol sólo crece por la raíz.
     * - No admite duplicados. La eliminación no forma parte del alcance.
     * - Cada nodo reserva una posición extra para alojar el desborde antes de dividirse.
     *
     * Al dividir una hoja la primera clave del hermano se COPIA al padre; al dividir un nodo
     * interno la clave del medio SUBE y no queda en ninguno de los hijos.
     *
     * | Operación | Mejor    | Peor         |
     * |-----------|----------|--------------|
     * | insert    | O(log n) | O(log n)     |
     * | find      | O(log n) | O(log n)     |
     * | range     | O(log n) | O(log n + k) |
     *
     * @tparam TKey clave con operator<.
     * @tparam TValue tipo apuntado (no propietario).
     * @tparam ORDER máximo de hijos por nodo interno (por defecto 4).
     */
    template<typename TKey, typename TValue, std::size_t ORDER = 4>
    class BPlusTree final
    {
        static_assert(ORDER >= 3, "El orden mínimo de un B+ es 3");

    public:
        static constexpr std::size_t MAX_KEYS {ORDER - 1};

        /// @brief Nodo interno u hoja.
        struct Node
        {
            bool isLeaf;
            std::size_t keyCount;
            TKey keys[ORDER];
            Node* pChildren[ORDER + 1];
            TValue* pValues[ORDER];
            Node* pNext;
        };

        /// @brief Resultado de una búsqueda puntual.
        struct Lookup
        {
            TValue* pValue;
            std::size_t visited;
        };

        BPlusTree() = default;

        ~BPlusTree()
        {
            destroy(m_pRoot);
        }

        BPlusTree(const BPlusTree&) = delete;
        BPlusTree& operator=(const BPlusTree&) = delete;

        /**
         * @brief Inserta la clave contemplando árbol vacío, hoja con espacio, división de hojas,
         * división de nodos internos y creación de una nueva raíz.
         * @return false si la clave ya existía.
         */
        bool insert(const TKey& key, TValue* pValue)
        {
            const bool inserted {find(key).pValue == nullptr};
            if (inserted && m_pRoot == nullptr)
            {
                m_pRoot = newNode(true);
                insertIntoLeaf(m_pRoot, key, pValue);
            }
            else if (inserted)
            {
                TKey promoted {};
                Node* pSibling = nullptr;
                insertInto(m_pRoot, key, pValue, promoted, pSibling);
                if (pSibling != nullptr)
                {
                    growNewRoot(promoted, pSibling);
                }
            }
            m_size += inserted ? 1 : 0;
            return inserted;
        }

        /// @brief Búsqueda puntual contando nodos visitados (internos más la hoja).
        Lookup find(const TKey& key) const
        {
            Lookup result {nullptr, 0};
            if (m_pRoot != nullptr)
            {
                const Node* pLeaf = descendToLeaf(key, result.visited);
                for (std::size_t i = 0; i < pLeaf->keyCount && result.pValue == nullptr; ++i)
                {
                    if (equals(pLeaf->keys[i], key))
                    {
                        result.pValue = pLeaf->pValues[i];
                    }
                }
            }
            return result;
        }

        /**
         * @brief Visita en orden ascendente las claves del intervalo [from, to].
         *
         * Desciende una única vez hasta la hoja de @p from y luego avanza por los enlaces
         * @c pNext sin recorrer el resto del árbol.
         * @return nodos visitados (descenso + hojas adicionales).
         */
        template<typename TVisitor>
        std::size_t range(const TKey& from, const TKey& to, TVisitor visit) const
        {
            std::size_t visited {0};
            if (m_pRoot != nullptr && !(to < from))
            {
                const Node* pLeaf = descendToLeaf(from, visited);
                bool done {false};
                while (pLeaf != nullptr && !done)
                {
                    for (std::size_t i = 0; i < pLeaf->keyCount && !done; ++i)
                    {
                        done = to < pLeaf->keys[i];
                        if (!done && !(pLeaf->keys[i] < from))
                        {
                            visit(pLeaf->keys[i], pLeaf->pValues[i]);
                        }
                    }
                    pLeaf = pLeaf->pNext;
                    visited += (pLeaf != nullptr && !done) ? 1 : 0;
                }
            }
            return visited;
        }

        /// @brief Recorre todas las claves en orden siguiendo los enlaces entre hojas.
        template<typename TVisitor>
        void forEachInOrder(TVisitor visit) const
        {
            for (const Node* pLeaf = leftmostLeaf(); pLeaf != nullptr; pLeaf = pLeaf->pNext)
            {
                for (std::size_t i = 0; i < pLeaf->keyCount; ++i)
                {
                    visit(pLeaf->keys[i], pLeaf->pValues[i]);
                }
            }
        }

        /// @brief Cantidad de niveles (0 vacío, 1 si la raíz es hoja).
        std::size_t levels() const
        {
            std::size_t count {0};
            for (const Node* pNode = m_pRoot; pNode != nullptr; pNode = pNode->isLeaf ? nullptr : pNode->pChildren[0])
            {
                ++count;
            }
            return count;
        }

        /// @brief Verifica recursivamente que todas las hojas estén a la misma profundidad.
        bool leavesAtSameDepth() const
        {
            long leafDepth {-1};
            return checkLeafDepth(m_pRoot, 0, leafDepth);
        }

        const Node* root() const noexcept
        {
            return m_pRoot;
        }

        std::size_t size() const noexcept
        {
            return m_size;
        }

        bool isEmpty() const noexcept
        {
            return m_pRoot == nullptr;
        }

    private:
        static bool equals(const TKey& left, const TKey& right)
        {
            return !(left < right) && !(right < left);
        }

        static Node* newNode(const bool isLeaf)
        {
            Node* pNode = new Node {};
            pNode->isLeaf = isLeaf;
            pNode->keyCount = 0;
            pNode->pNext = nullptr;
            for (std::size_t i = 0; i <= ORDER; ++i)
            {
                pNode->pChildren[i] = nullptr;
            }
            for (std::size_t i = 0; i < ORDER; ++i)
            {
                pNode->pValues[i] = nullptr;
            }
            return pNode;
        }

        static std::size_t childIndex(const Node* pNode, const TKey& key)
        {
            std::size_t index {0};
            while (index < pNode->keyCount && !(key < pNode->keys[index]))
            {
                ++index;
            }
            return index;
        }

        const Node* descendToLeaf(const TKey& key, std::size_t& visited) const
        {
            const Node* pNode = m_pRoot;
            ++visited;
            while (!pNode->isLeaf)
            {
                pNode = pNode->pChildren[childIndex(pNode, key)];
                ++visited;
            }
            return pNode;
        }

        const Node* leftmostLeaf() const
        {
            const Node* pNode = m_pRoot;
            while (pNode != nullptr && !pNode->isLeaf)
            {
                pNode = pNode->pChildren[0];
            }
            return pNode;
        }

        void insertInto(Node* pNode, const TKey& key, TValue* pValue, TKey& promoted, Node*& pSibling)
        {
            if (pNode->isLeaf)
            {
                insertIntoLeaf(pNode, key, pValue);
                if (pNode->keyCount > MAX_KEYS)
                {
                    splitLeaf(pNode, promoted, pSibling);
                }
            }
            else
            {
                const std::size_t index {childIndex(pNode, key)};
                TKey childPromoted {};
                Node* pChildSibling = nullptr;
                insertInto(pNode->pChildren[index], key, pValue, childPromoted, pChildSibling);
                if (pChildSibling != nullptr)
                {
                    insertIntoInternal(pNode, index, childPromoted, pChildSibling);
                    if (pNode->keyCount > MAX_KEYS)
                    {
                        splitInternal(pNode, promoted, pSibling);
                    }
                }
            }
        }

        static void insertIntoLeaf(Node* pLeaf, const TKey& key, TValue* pValue)
        {
            std::size_t i {pLeaf->keyCount};
            while (i > 0 && key < pLeaf->keys[i - 1])
            {
                pLeaf->keys[i] = pLeaf->keys[i - 1];
                pLeaf->pValues[i] = pLeaf->pValues[i - 1];
                --i;
            }
            pLeaf->keys[i] = key;
            pLeaf->pValues[i] = pValue;
            ++pLeaf->keyCount;
        }

        static void insertIntoInternal(Node* pNode, const std::size_t index, const TKey& key, Node* pRightChild)
        {
            for (std::size_t i = pNode->keyCount; i > index; --i)
            {
                pNode->keys[i] = pNode->keys[i - 1];
                pNode->pChildren[i + 1] = pNode->pChildren[i];
            }
            pNode->keys[index] = key;
            pNode->pChildren[index + 1] = pRightChild;
            ++pNode->keyCount;
        }

        static void splitLeaf(Node* pLeaf, TKey& promoted, Node*& pSibling)
        {
            const std::size_t leftCount {pLeaf->keyCount / 2};
            Node* pRight = newNode(true);
            for (std::size_t i = leftCount; i < pLeaf->keyCount; ++i)
            {
                pRight->keys[i - leftCount] = pLeaf->keys[i];
                pRight->pValues[i - leftCount] = pLeaf->pValues[i];
                pLeaf->pValues[i] = nullptr;
            }
            pRight->keyCount = pLeaf->keyCount - leftCount;
            pLeaf->keyCount = leftCount;
            pRight->pNext = pLeaf->pNext;
            pLeaf->pNext = pRight;
            promoted = pRight->keys[0];
            pSibling = pRight;
        }

        static void splitInternal(Node* pNode, TKey& promoted, Node*& pSibling)
        {
            const std::size_t middle {pNode->keyCount / 2};
            Node* pRight = newNode(false);
            std::size_t target {0};
            for (std::size_t i = middle + 1; i < pNode->keyCount; ++i, ++target)
            {
                pRight->keys[target] = pNode->keys[i];
                pRight->pChildren[target] = pNode->pChildren[i];
                pNode->pChildren[i] = nullptr;
            }
            pRight->pChildren[target] = pNode->pChildren[pNode->keyCount];
            pNode->pChildren[pNode->keyCount] = nullptr;
            pRight->keyCount = target;
            promoted = pNode->keys[middle];
            pNode->keyCount = middle;
            pSibling = pRight;
        }

        void growNewRoot(const TKey& separator, Node* pRightChild)
        {
            Node* pNewRoot = newNode(false);
            pNewRoot->keys[0] = separator;
            pNewRoot->pChildren[0] = m_pRoot;
            pNewRoot->pChildren[1] = pRightChild;
            pNewRoot->keyCount = 1;
            m_pRoot = pNewRoot;
        }

        static bool checkLeafDepth(const Node* pNode, const long depth, long& leafDepth)
        {
            bool consistent {true};
            if (pNode != nullptr && pNode->isLeaf)
            {
                leafDepth = leafDepth == -1 ? depth : leafDepth;
                consistent = leafDepth == depth;
            }
            else if (pNode != nullptr)
            {
                for (std::size_t i = 0; i <= pNode->keyCount && consistent; ++i)
                {
                    consistent = checkLeafDepth(pNode->pChildren[i], depth + 1, leafDepth);
                }
            }
            return consistent;
        }

        static void destroy(Node* pNode) noexcept
        {
            if (pNode != nullptr)
            {
                for (std::size_t i = 0; !pNode->isLeaf && i <= pNode->keyCount; ++i)
                {
                    destroy(pNode->pChildren[i]);
                }
                delete pNode;
            }
        }

        Node* m_pRoot {nullptr};
        std::size_t m_size {0};
    };

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_B_PLUS_TREE_HPP
