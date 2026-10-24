#ifndef HUBFLOW_SORTING_DOMAIN_VALUEOBJECTS_SORT_CRITERIA_HPP
#define HUBFLOW_SORTING_DOMAIN_VALUEOBJECTS_SORT_CRITERIA_HPP

#include <cstdint>

namespace hubflow::sorting::domain
{

    /// @brief Criterio por el que está ordenado un lote.
    enum class SortCriteria : std::uint8_t
    {
        None,
        CodeAscending,
        WeightDescending
    };

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_VALUEOBJECTS_SORT_CRITERIA_HPP
