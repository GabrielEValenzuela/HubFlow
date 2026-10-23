#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_GET_ACTIVE_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_GET_ACTIVE_SORTING_BATCH_HPP

#include "hubflow/sorting/application/ports/in/getActiveSortingBatchUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/// @brief Implementación de GetActiveSortingBatchUseCase.
class GetActiveSortingBatch final : public GetActiveSortingBatchUseCase
{
public:
    explicit GetActiveSortingBatch(const domain::SortingDesk& desk);

private:
    const domain::SortingBatch* execute(const shared::application::Principal& principal) const override;

    const domain::SortingDesk& m_desk;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_QUERIES_GET_ACTIVE_SORTING_BATCH_HPP
