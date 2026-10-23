#include "hubflow/sorting/application/usecases/commands/prepareSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

PrepareSortingBatch::PrepareSortingBatch(domain::SortingDesk& desk)
    : m_desk {desk}
{
}

const domain::SortingBatch& PrepareSortingBatch::execute(const shared::application::Principal& principal,
                                                         std::uint64_t                         batchId)
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    domain::SortingBatch& batch = m_desk.require(batchId);
    batch.prepare();
    return batch;
}

} // namespace hubflow::sorting::application
