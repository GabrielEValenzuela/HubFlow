#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_BINARY_SEARCH_IN_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_BINARY_SEARCH_IN_SORTING_BATCH_HPP

#include "hubflow/sorting/application/ports/in/binarySearchInSortingBatchUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/// @brief Implementación de BinarySearchInSortingBatchUseCase.
class BinarySearchInSortingBatch final : public BinarySearchInSortingBatchUseCase
{
public:
    explicit BinarySearchInSortingBatch(const domain::SortingDesk& desk);

private:
    domain::BatchSearchOutcome execute(const shared::application::Principal& principal,
                                       std::uint64_t                         batchId,
                                       const std::string&                    code) const override;

    const domain::SortingDesk& m_desk;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_BINARY_SEARCH_IN_SORTING_BATCH_HPP
