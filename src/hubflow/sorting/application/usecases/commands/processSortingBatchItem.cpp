#include "hubflow/sorting/application/usecases/commands/processSortingBatchItem.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

ProcessSortingBatchItem::ProcessSortingBatchItem(domain::SortingDesk& desk)
    : m_desk {desk}
{
}

const logistics::domain::Shipment& ProcessSortingBatchItem::execute(const shared::application::Principal& principal,
                                                                    std::uint64_t                         batchId)
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    return m_desk.require(batchId).processNext();
}

} // namespace hubflow::sorting::application
