#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_SORT_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_SORT_SORTING_BATCH_HPP

#include "hubflow/sorting/application/ports/in/sortSortingBatchUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

    /// @brief Implementación de SortSortingBatchUseCase.
    class SortSortingBatch final : public SortSortingBatchUseCase
    {
    public:
        explicit SortSortingBatch(domain::SortingDesk& desk);

    private:
        domain::SortStats execute(const shared::application::Principal& principal,
                                  std::uint64_t batchId,
                                  domain::SortCriteria criteria) override;

        domain::SortingDesk& m_desk;
    };

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_SORT_SORTING_BATCH_HPP
