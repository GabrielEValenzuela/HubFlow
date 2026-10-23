#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_OPEN_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_OPEN_SORTING_BATCH_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/sorting/application/ports/in/openSortingBatchUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/// @brief Implementación de OpenSortingBatchUseCase.
class OpenSortingBatch final : public OpenSortingBatchUseCase
{
public:
    OpenSortingBatch(const logistics::application::LogisticsModel& model, domain::SortingDesk& desk);

private:
    const domain::SortingBatch& execute(const shared::application::Principal& principal,
                                        logistics::domain::Zone               zone) override;

    const logistics::application::LogisticsModel& m_model;
    domain::SortingDesk&                          m_desk;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_OPEN_SORTING_BATCH_HPP
