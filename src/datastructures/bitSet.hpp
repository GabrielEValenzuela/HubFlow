#ifndef HUBFLOW_DATASTRUCTURES_BIT_SET_HPP
#define HUBFLOW_DATASTRUCTURES_BIT_SET_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace hubflow::ds
{

    /**
     * @brief Conjunto de @p BITS bits (1..32) almacenado en una palabra sin signo.
     *
     * | Operación          | Complejidad                          |
     * |--------------------|--------------------------------------|
     * | set / unset / isSet | O(1)                                 |
     * | activeCount           | O(k), k = bits activos (Kernighan)   |
     * | isAnySet / isNoneSet         | O(1)                                 |
     * | AND / OR / XOR     | O(1): operan sobre la palabra entera |
     *
     * @tparam BITS cantidad de posiciones.
     */
    template<std::size_t BITS>
    class BitSet final
    {
        static_assert(BITS > 0 && BITS <= 32, "BitSet soporta entre 1 y 32 bits");

    public:
        BitSet() = default;

        /// @brief Construye desde una palabra cruda descartando bits fuera de rango.
        static BitSet fromWord(const std::uint32_t word) noexcept
        {
            BitSet bitSet;
            bitSet.m_word = word & mask();
            return bitSet;
        }

        /// @throws std::out_of_range si @p position >= BITS.
        void set(const std::size_t position)
        {
            m_word |= bit(position);
        }

        /// @throws std::out_of_range si @p position >= BITS.
        void unset(const std::size_t position)
        {
            m_word &= ~bit(position);
        }

        /// @throws std::out_of_range si @p position >= BITS.
        bool isSet(const std::size_t position) const
        {
            return (m_word & bit(position)) != 0;
        }

        /// @brief Cantidad de bits activos (algoritmo de Kernighan).
        std::size_t activeCount() const noexcept
        {
            std::size_t active {0};
            for (std::uint32_t rest = m_word; rest != 0; rest &= rest - 1)
            {
                ++active;
            }
            return active;
        }

        bool isAnySet() const noexcept
        {
            return m_word != 0;
        }

        bool isNoneSet() const noexcept
        {
            return m_word == 0;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            return fromWord(m_word & other.m_word);
        }

        BitSet operator|(const BitSet& other) const noexcept
        {
            return fromWord(m_word | other.m_word);
        }

        BitSet operator^(const BitSet& other) const noexcept
        {
            return fromWord(m_word ^ other.m_word);
        }

        bool operator==(const BitSet& other) const noexcept
        {
            return m_word == other.m_word;
        }

        std::uint32_t toWord() const noexcept
        {
            return m_word;
        }

        static constexpr std::size_t size() noexcept
        {
            return BITS;
        }

    private:
        static constexpr std::uint32_t mask() noexcept
        {
            return BITS == 32 ? 0xFFFFFFFFU : ((1U << BITS) - 1U);
        }

        static std::uint32_t bit(const std::size_t position)
        {
            if (position >= BITS)
            {
                throw std::out_of_range("Posición de bit inválida");
            }
            return 1U << position;
        }

        std::uint32_t m_word {0};
    };

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_BIT_SET_HPP
