#include "hubflow/sorting/application/usecases/commands/returnProcessedItem.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

ReturnProcessedItem::ReturnProcessedItem(domain::SortingDesk& desk)
    : m_desk {desk}
{
}

const logistics::domain::Shipment& ReturnProcessedItem::execute(const shared::application::Principal& principal,
                                                                std::uint64_t                         batchId,
                                                                const std::string&                    code)
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    return m_desk.require(batchId).returnProcessed(code);
}

} // namespace hubflow::sorting::application
