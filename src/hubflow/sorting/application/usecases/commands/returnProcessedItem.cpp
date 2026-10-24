#include "hubflow/sorting/application/usecases/commands/returnProcessedItem.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

    ReturnProcessedItem::ReturnProcessedItem(domain::SortingDesk& desk,
                                             const logistics::domain::ShipmentGroupBook& groups)
        : m_desk {desk}
        , m_groups {groups}
    {
    }

    ds::DynamicArray<logistics::domain::Shipment*> ReturnProcessedItem::execute(
        const shared::application::Principal& principal, std::uint64_t batchId, const std::string& code)
    {
        principal.require(identity::domain::Permission::OperateSortingDesk);
        return m_desk.require(batchId).returnProcessed(code, m_groups);
    }

} // namespace hubflow::sorting::application
