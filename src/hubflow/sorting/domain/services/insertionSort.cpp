#include "hubflow/sorting/domain/services/insertionSort.hpp"

namespace hubflow::sorting::domain
{

    SortStats
    insertionSort(logistics::domain::Shipment** pItems, const std::size_t size, const ShipmentOrdering& ordering)
    {
        SortStats stats;
        for (std::size_t i = 1; i < size; ++i)
        {
            logistics::domain::Shipment* pCurrent = pItems[i];
            std::size_t hole {i};
            bool placed {false};
            while (hole > 0 && !placed)
            {
                ++stats.comparisons;
                placed = !ordering.precedes(*pCurrent, *pItems[hole - 1]);
                if (!placed)
                {
                    pItems[hole] = pItems[hole - 1];
                    ++stats.shifts;
                    --hole;
                }
            }
            pItems[hole] = pCurrent;
        }
        return stats;
    }

} // namespace hubflow::sorting::domain
