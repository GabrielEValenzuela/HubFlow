#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_LINEAR_SEARCH_IN_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_LINEAR_SEARCH_IN_SORTING_BATCH_HPP

#include "hubflow/sorting/application/ports/in/linearSearchInSortingBatchUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/// @brief Implementación de LinearSearchInSortingBatchUseCase.
class LinearSearchInSortingBatch final : public LinearSearchInSortingBatchUseCase
{
public:
    explicit LinearSearchInSortingBatch(const domain::SortingDesk& desk);

private:
    domain::BatchSearchOutcome execute(const shared::application::Principal& principal,
                                       std::uint64_t                         batchId,
                                       const std::string&                    code) const override;

    const domain::SortingDesk& m_desk;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_LINEAR_SEARCH_IN_SORTING_BATCH_HPP
