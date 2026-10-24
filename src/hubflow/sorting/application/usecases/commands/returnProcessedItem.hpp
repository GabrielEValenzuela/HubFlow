#ifndef HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_RETURN_PROCESSED_ITEM_HPP
#define HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_RETURN_PROCESSED_ITEM_HPP

#include "hubflow/sorting/application/ports/in/returnProcessedItemUseCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

    /// @brief Implementación de ReturnProcessedItemUseCase.
    class ReturnProcessedItem final : public ReturnProcessedItemUseCase
    {
    public:
        ReturnProcessedItem(domain::SortingDesk& desk, const logistics::domain::ShipmentGroupBook& groups);

    private:
        ds::DynamicArray<logistics::domain::Shipment*> execute(const shared::application::Principal& principal,
                                                               std::uint64_t batchId,
                                                               const std::string& code) override;

        domain::SortingDesk& m_desk;
        const logistics::domain::ShipmentGroupBook& m_groups;
    };

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_USECASES_COMMANDS_RETURN_PROCESSED_ITEM_HPP
