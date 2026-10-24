#ifndef HUBFLOW_DATASTRUCTURES_DYNAMIC_ARRAY_HPP
#define HUBFLOW_DATASTRUCTURES_DYNAMIC_ARRAY_HPP

#include <cstddef>
#include <stdexcept>

namespace hubflow::ds
{

    /**
     * @brief Arreglo redimensionable sobre @c new[] / @c delete[].
     *
     * pushBack y popBack son O(1) amortizado (la capacidad se duplica al llenarse); el acceso por
     * índice es O(1) y valida el rango.
     *
     * @tparam TElement tipo almacenado (construible por defecto y copiable).
     */
    template<typename TElement>
    class DynamicArray final
    {
    public:
        DynamicArray() = default;

        /// @brief Crea @p initialSize elementos con el valor @p fill.
        explicit DynamicArray(const std::size_t initialSize, const TElement& fill = TElement {})
            : m_pData {initialSize > 0 ? new TElement[initialSize] : nullptr}
            , m_size {initialSize}
            , m_capacity {initialSize}
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                m_pData[i] = fill;
            }
        }

        ~DynamicArray()
        {
            delete[] m_pData;
        }

        DynamicArray(const DynamicArray& other)
            : m_pData {other.m_capacity > 0 ? new TElement[other.m_capacity] : nullptr}
            , m_size {other.m_size}
            , m_capacity {other.m_capacity}
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                m_pData[i] = other.m_pData[i];
            }
        }

        DynamicArray& operator=(const DynamicArray& other)
        {
            if (this != &other)
            {
                DynamicArray copy {other};
                swap(copy);
            }
            return *this;
        }

        DynamicArray(DynamicArray&& other) noexcept
            : m_pData {other.m_pData}
            , m_size {other.m_size}
            , m_capacity {other.m_capacity}
        {
            other.m_pData = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }

        DynamicArray& operator=(DynamicArray&& other) noexcept
        {
            if (this != &other)
            {
                delete[] m_pData;
                m_pData = other.m_pData;
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                other.m_pData = nullptr;
                other.m_size = 0;
                other.m_capacity = 0;
            }
            return *this;
        }

        void pushBack(const TElement& value)
        {
            if (m_size == m_capacity)
            {
                grow();
            }
            m_pData[m_size++] = value;
        }

        /// @throws std::underflow_error si está vacío.
        void popBack()
        {
            if (m_size == 0)
            {
                throw std::underflow_error("DynamicArray vacío");
            }
            --m_size;
        }

        /// @throws std::out_of_range si @p index >= size().
        TElement& operator[](const std::size_t index)
        {
            return m_pData[checked(index)];
        }

        /// @throws std::out_of_range si @p index >= size().
        const TElement& operator[](const std::size_t index) const
        {
            return m_pData[checked(index)];
        }

        void clear() noexcept
        {
            m_size = 0;
        }

        std::size_t size() const noexcept
        {
            return m_size;
        }

        bool isEmpty() const noexcept
        {
            return m_size == 0;
        }

    private:
        static constexpr std::size_t INITIAL_CAPACITY {4};

        std::size_t checked(const std::size_t index) const
        {
            if (index >= m_size)
            {
                throw std::out_of_range("Índice fuera de rango");
            }
            return index;
        }

        void grow()
        {
            const std::size_t newCapacity {m_capacity == 0 ? INITIAL_CAPACITY : m_capacity * 2};
            TElement* pBigger = new TElement[newCapacity];
            for (std::size_t i = 0; i < m_size; ++i)
            {
                pBigger[i] = m_pData[i];
            }
            delete[] m_pData;
            m_pData = pBigger;
            m_capacity = newCapacity;
        }

        void swap(DynamicArray& other) noexcept
        {
            TElement* pData {m_pData};
            const std::size_t size {m_size};
            const std::size_t capacity {m_capacity};
            m_pData = other.m_pData;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_pData = pData;
            other.m_size = size;
            other.m_capacity = capacity;
        }

        TElement* m_pData {nullptr};
        std::size_t m_size {0};
        std::size_t m_capacity {0};
    };

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_DYNAMIC_ARRAY_HPP
