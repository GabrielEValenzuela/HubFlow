#include "hubflow/sorting/application/usecases/commands/sortSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/sorting/domain/services/shipmentOrdering.hpp"

namespace hubflow::sorting::application
{

    SortSortingBatch::SortSortingBatch(domain::SortingDesk& desk)
        : m_desk {desk}
    {
    }

    domain::SortStats SortSortingBatch::execute(const shared::application::Principal& principal,
                                                std::uint64_t batchId,
                                                domain::SortCriteria criteria)
    {
        principal.require(identity::domain::Permission::OperateSortingDesk);
        return m_desk.require(batchId).sort(domain::ShipmentOrdering::of(criteria));
    }

} // namespace hubflow::sorting::application
