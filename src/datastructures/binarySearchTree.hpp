#ifndef HUBFLOW_DATASTRUCTURES_BINARY_SEARCH_TREE_HPP
#define HUBFLOW_DATASTRUCTURES_BINARY_SEARCH_TREE_HPP

#include <cstddef>

namespace hubflow::ds
{

/// @brief Decisión tomada en un nodo durante una búsqueda en el BST.
enum class BstStep
{
    Left,
    Right,
    Found
};

/**
 * @brief Árbol binario de búsqueda (BST/ABB) que guarda punteros NO propietarios.
 *
 * - Claves menores a la izquierda y mayores a la derecha; no admite duplicados.
 * - Inserción iterativa; recorrido in-order y altura recursivos.
 * - Convención de altura: vacío = -1, un nodo = 0, cada nivel adicional suma 1.
 * - El destructor libera los nodos y nunca ejecuta @c delete sobre @c TValue*.
 *
 * | Operación | Mejor | Esperado (balanceado) | Peor (degenerado) |
 * |-----------|-------|-----------------------|-------------------|
 * | insert    | O(1)  | O(log n)              | O(n)              |
 * | find      | O(1)  | O(log n)              | O(n)              |
 * | inOrder   | O(n)  | O(n)                  | O(n)              |
 * | height    | O(n)  | O(n)                  | O(n)              |
 *
 * Insertar claves ya ordenadas degenera el árbol en una lista (altura n-1).
 *
 * @tparam TKey clave con operator<.
 * @tparam TValue tipo apuntado (no propietario).
 */
template <typename TKey, typename TValue>
class BinarySearchTree final
{
public:
    /// @brief Nodo con hijos izquierdo y derecho.
    struct Node
    {
        TKey    key;
        TValue* pValue;
        Node*   pLeft;
        Node*   pRight;
    };

    /// @brief Resultado de una búsqueda: valor (o nullptr) y nodos visitados.
    struct Lookup
    {
        TValue*     pValue;
        std::size_t visited;
    };

    BinarySearchTree() = default;

    ~BinarySearchTree()
    {
        destroy(m_pRoot);
    }

    BinarySearchTree(const BinarySearchTree&)            = delete;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;

    /**
     * @brief Inserción iterativa.
     * @return false si la clave ya existía (rechaza duplicados).
     */
    bool insert(const TKey& key, TValue* pValue)
    {
        Node** ppLink = &m_pRoot;
        bool   duplicated {false};
        while (*ppLink != nullptr && !duplicated)
        {
            if (key < (*ppLink)->key)
            {
                ppLink = &(*ppLink)->pLeft;
            }
            else if ((*ppLink)->key < key)
            {
                ppLink = &(*ppLink)->pRight;
            }
            else
            {
                duplicated = true;
            }
        }
        if (!duplicated)
        {
            *ppLink = new Node {key, pValue, nullptr, nullptr};
            ++m_size;
        }
        return !duplicated;
    }

    /**
     * @brief Busca @p key notificando cada nodo visitado y la decisión tomada.
     * @param key clave buscada.
     * @param onVisit callable (const TKey&, BstStep).
     */
    template <typename TVisitor>
    Lookup find(const TKey& key, TVisitor onVisit) const
    {
        Lookup result {nullptr, 0};
        Node*  pCurrent = m_pRoot;
        while (pCurrent != nullptr)
        {
            ++result.visited;
            if (key < pCurrent->key)
            {
                onVisit(pCurrent->key, BstStep::Left);
                pCurrent = pCurrent->pLeft;
            }
            else if (pCurrent->key < key)
            {
                onVisit(pCurrent->key, BstStep::Right);
                pCurrent = pCurrent->pRight;
            }
            else
            {
                onVisit(pCurrent->key, BstStep::Found);
                result.pValue = pCurrent->pValue;
                pCurrent      = nullptr;
            }
        }
        return result;
    }

    /// @brief Busca @p key sin observar el recorrido.
    Lookup find(const TKey& key) const
    {
        return find(key, [](const TKey&, BstStep) {});
    }

    /**
     * @brief Recorrido in-order recursivo (izquierdo, actual, derecho).
     *
     * Por la propiedad del BST, todo el subárbol izquierdo es menor que el nodo y todo el
     * derecho mayor, por lo que las claves se visitan en orden ascendente.
     */
    template <typename TVisitor>
    void inOrder(TVisitor visit) const
    {
        inOrder(m_pRoot, visit);
    }

    /// @brief Altura recursiva (vacío = -1, hoja = 0).
    int height() const
    {
        return height(m_pRoot);
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
    template <typename TVisitor>
    static void inOrder(const Node* pNode, TVisitor& visit)
    {
        if (pNode != nullptr)
        {
            inOrder(pNode->pLeft, visit);
            visit(pNode->key, pNode->pValue);
            inOrder(pNode->pRight, visit);
        }
    }

    static int height(const Node* pNode)
    {
        int result {-1};
        if (pNode != nullptr)
        {
            const int left {height(pNode->pLeft)};
            const int right {height(pNode->pRight)};
            result = 1 + (left > right ? left : right);
        }
        return result;
    }

    static void destroy(Node* pNode) noexcept
    {
        if (pNode != nullptr)
        {
            destroy(pNode->pLeft);
            destroy(pNode->pRight);
            delete pNode;
        }
    }

    Node*       m_pRoot {nullptr};
    std::size_t m_size {0};
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_BINARY_SEARCH_TREE_HPP
