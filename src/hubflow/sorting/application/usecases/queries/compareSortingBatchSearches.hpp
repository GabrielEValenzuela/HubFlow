#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_COMPARE_SORTING_BATCH_SEARCHES_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_COMPARE_SORTING_BATCH_SEARCHES_HPP

#include "hubflow/sorting/application/ports/in/compareSortingBatchSearchesUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

    /// @brief Implementación de CompareSortingBatchSearchesUseCase.
    class CompareSortingBatchSearches final : public CompareSortingBatchSearchesUseCase
    {
    public:
        explicit CompareSortingBatchSearches(const domain::SortingDesk& desk);

    private:
        SearchComparison execute(const shared::application::Principal& principal,
                                 std::uint64_t batchId,
                                 const std::string& code) const override;

        const domain::SortingDesk& m_desk;
    };

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_COMPARE_SORTING_BATCH_SEARCHES_HPP
