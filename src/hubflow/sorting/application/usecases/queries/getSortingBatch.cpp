#include "hubflow/sorting/application/usecases/queries/getSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

GetSortingBatch::GetSortingBatch(const domain::SortingDesk& desk)
    : m_desk {desk}
{
}

const domain::SortingBatch& GetSortingBatch::execute(const shared::application::Principal& principal,
                                                     std::uint64_t                         batchId) const
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    return m_desk.require(batchId);
}

} // namespace hubflow::sorting::application
