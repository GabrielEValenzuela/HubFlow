#include "hubflow/sorting/application/usecases/queries/binarySearchInSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{
namespace
{
const domain::BinaryBatchSearch gs_binarySearchImpl;
const domain::BatchSearch&      gs_binarySearch {gs_binarySearchImpl};
} // namespace

BinarySearchInSortingBatch::BinarySearchInSortingBatch(const domain::SortingDesk& desk)
    : m_desk {desk}
{
}

domain::BatchSearchOutcome BinarySearchInSortingBatch::execute(const shared::application::Principal& principal,
                                                               std::uint64_t                         batchId,
                                                               const std::string&                    code) const
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    return gs_binarySearch.search(m_desk.require(batchId), code);
}

} // namespace hubflow::sorting::application
