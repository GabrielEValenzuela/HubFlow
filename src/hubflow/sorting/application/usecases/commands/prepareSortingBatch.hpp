#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_PREPARE_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_PREPARE_SORTING_BATCH_HPP

#include "hubflow/sorting/application/ports/in/prepareSortingBatchUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/// @brief Implementación de PrepareSortingBatchUseCase.
class PrepareSortingBatch final : public PrepareSortingBatchUseCase
{
public:
    explicit PrepareSortingBatch(domain::SortingDesk& desk);

private:
    const domain::SortingBatch& execute(const shared::application::Principal& principal,
                                        std::uint64_t                         batchId) override;

    domain::SortingDesk& m_desk;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_PREPARE_SORTING_BATCH_HPP
