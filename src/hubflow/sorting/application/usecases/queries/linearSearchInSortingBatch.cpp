#include "hubflow/sorting/application/usecases/queries/linearSearchInSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{
namespace
{
const domain::LinearBatchSearch gs_linearSearchImpl;
const domain::BatchSearch&      gs_linearSearch {gs_linearSearchImpl};
} // namespace

LinearSearchInSortingBatch::LinearSearchInSortingBatch(const domain::SortingDesk& desk)
    : m_desk {desk}
{
}

domain::BatchSearchOutcome LinearSearchInSortingBatch::execute(const shared::application::Principal& principal,
                                                               std::uint64_t                         batchId,
                                                               const std::string&                    code) const
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    return gs_linearSearch.search(m_desk.require(batchId), code);
}

} // namespace hubflow::sorting::application
