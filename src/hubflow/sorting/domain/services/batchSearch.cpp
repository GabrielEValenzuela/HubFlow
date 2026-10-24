#include "hubflow/sorting/domain/services/batchSearch.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::sorting::domain
{

    BatchSearchOutcome LinearBatchSearch::search(const SortingBatch& batch, const std::string& code) const
    {
        BatchSearchOutcome outcome;
        for (std::size_t i = 0; i < batch.size() && !outcome.position.has_value(); ++i)
        {
            ++outcome.comparisons;
            if (batch.shipmentAt(i).code() == code)
            {
                outcome.position = i;
            }
        }
        return outcome;
    }

    BatchSearchOutcome BinaryBatchSearch::search(const SortingBatch& batch, const std::string& code) const
    {
        if (batch.sortedBy() != SortCriteria::CodeAscending)
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::InvalidState,
                                                 "sorting.binary_requires_code_order",
                                                 "La búsqueda binaria requiere el lote ordenado por código"};
        }
        BatchSearchOutcome outcome;
        std::size_t left {0};
        std::size_t right {batch.size()};
        while (left < right && !outcome.position.has_value())
        {
            const std::size_t middle {left + (right - left) / 2};
            const std::string& candidate = batch.shipmentAt(middle).code();
            outcome.steps.pushBack(BinarySearchStep {left, right - 1, middle, candidate});
            ++outcome.comparisons;
            if (candidate == code)
            {
                outcome.position = middle;
            }
            else if (candidate < code)
            {
                left = middle + 1;
            }
            else
            {
                right = middle;
            }
        }
        return outcome;
    }

} // namespace hubflow::sorting::domain
