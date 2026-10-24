#ifndef HUBFLOW_DATASTRUCTURES_STACK_HPP
#define HUBFLOW_DATASTRUCTURES_STACK_HPP

#include <cstddef>
#include <stdexcept>

namespace hubflow::ds
{

    /**
     * @brief Pila LIFO enlazada.
     *
     * @c m_pTop apunta al último elemento apilado; push, pop y top son O(1).
     * Ownership: libera sólo sus nodos; si @p TElement es puntero nunca ejecuta @c delete sobre él.
     *
     * @tparam TElement tipo almacenado (copiable).
     */
    template<typename TElement>
    class Stack final
    {
    public:
        /// @brief Nodo con enlace al elemento de abajo.
        struct Node
        {
            TElement value;
            Node* pBelow;
        };

        Stack() = default;

        ~Stack()
        {
            clear();
        }

        Stack(const Stack&) = delete;
        Stack& operator=(const Stack&) = delete;

        /// @brief Apila. O(1).
        void push(const TElement& value)
        {
            // cppcheck-suppress publicAllocationError
            m_pTop = new Node {value, m_pTop};
            ++m_size;
        }

        /**
         * @brief Desapila y devuelve el tope. O(1).
         * @throws std::underflow_error si la pila está vacía.
         */
        TElement pop()
        {
            requireElements();
            Node* pOld = m_pTop;
            const TElement value {pOld->value};
            m_pTop = pOld->pBelow;
            delete pOld;
            --m_size;
            return value;
        }

        /**
         * @brief Tope sin desapilar. O(1).
         * @throws std::underflow_error si la pila está vacía.
         */
        const TElement& top() const
        {
            requireElements();
            return m_pTop->value;
        }

        /// @brief Recorre desde el tope hacia la base.
        template<typename TVisitor>
        void forEach(TVisitor visit) const
        {
            for (Node* pCurrent = m_pTop; pCurrent != nullptr; pCurrent = pCurrent->pBelow)
            {
                visit(pCurrent->value);
            }
        }

        /// @brief Libera todos los nodos.
        void clear() noexcept
        {
            while (m_pTop != nullptr)
            {
                Node* pBelow = m_pTop->pBelow;
                delete m_pTop;
                m_pTop = pBelow;
            }
            m_size = 0;
        }

        bool isEmpty() const noexcept
        {
            return m_pTop == nullptr;
        }

        std::size_t size() const noexcept
        {
            return m_size;
        }

    private:
        void requireElements() const
        {
            if (m_pTop == nullptr)
            {
                throw std::underflow_error("Stack vacía");
            }
        }

        Node* m_pTop {nullptr};
        std::size_t m_size {0};
    };

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_STACK_HPP
