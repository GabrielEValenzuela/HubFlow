#ifndef HUBFLOW_DATASTRUCTURES_DOUBLY_LINKED_LIST_HPP
#define HUBFLOW_DATASTRUCTURES_DOUBLY_LINKED_LIST_HPP

#include <cstddef>

namespace hubflow::ds
{

/**
 * @brief Lista doblemente enlazada con punteros a cabeza y cola.
 *
 * El recorrido inverso usa exclusivamente los enlaces @c pPrev: no copia los elementos a otra
 * estructura. Ownership: libera sólo sus nodos.
 *
 * | Operación      | Complejidad |
 * |----------------|-------------|
 * | pushBack       | O(1)        |
 * | forEach        | O(n)        |
 * | forEachReverse | O(n)        |
 *
 * @tparam TElement tipo almacenado (copiable).
 */
template <typename TElement>
class DoublyLinkedList final
{
public:
    /// @brief Nodo con enlaces al anterior y al siguiente.
    struct Node
    {
        TElement value;
        Node*    pPrev;
        Node*    pNext;
    };

    DoublyLinkedList() = default;

    ~DoublyLinkedList()
    {
        clear();
    }

    DoublyLinkedList(const DoublyLinkedList&)            = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    /// @brief Agrega al final. O(1).
    void pushBack(const TElement& value)
    {
        Node* pNode = new Node {value, m_pTail, nullptr};
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

    /// @brief Recorre del más antiguo al más reciente siguiendo @c pNext.
    template <typename TVisitor>
    void forEach(TVisitor visit) const
    {
        for (Node* pCurrent = m_pHead; pCurrent != nullptr; pCurrent = pCurrent->pNext)
        {
            visit(pCurrent->value);
        }
    }

    /// @brief Recorre del más reciente al más antiguo siguiendo @c pPrev.
    template <typename TVisitor>
    void forEachReverse(TVisitor visit) const
    {
        for (Node* pCurrent = m_pTail; pCurrent != nullptr; pCurrent = pCurrent->pPrev)
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

    const Node* head() const noexcept
    {
        return m_pHead;
    }

    const Node* tail() const noexcept
    {
        return m_pTail;
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
    Node*       m_pHead {nullptr};
    Node*       m_pTail {nullptr};
    std::size_t m_size {0};
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_DOUBLY_LINKED_LIST_HPP
