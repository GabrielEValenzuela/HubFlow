#include "hubflow/sorting/application/usecases/commands/processSortingBatchItem.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

ProcessSortingBatchItem::ProcessSortingBatchItem(domain::SortingDesk&                        desk,
                                                 const logistics::domain::ShipmentGroupBook& groups)
    : m_desk {desk}
    , m_groups {groups}
{
}

ds::DynamicArray<logistics::domain::Shipment*>
ProcessSortingBatchItem::execute(const shared::application::Principal& principal,
                                 const std::uint64_t                   batchId,
                                 const ds::DynamicArray<std::string>&  codes)
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    domain::SortingBatch& batch = m_desk.require(batchId);
    return codes.isEmpty() ? batch.processNext(m_groups) : batch.processByCodes(codes, m_groups);
}

} // namespace hubflow::sorting::application
