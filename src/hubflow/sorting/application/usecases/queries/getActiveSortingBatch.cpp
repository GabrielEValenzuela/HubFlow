#include "hubflow/sorting/application/usecases/queries/getActiveSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

    GetActiveSortingBatch::GetActiveSortingBatch(const domain::SortingDesk& desk)
        : m_desk {desk}
    {
    }

    const domain::SortingBatch* GetActiveSortingBatch::execute(const shared::application::Principal& principal) const
    {
        principal.require(identity::domain::Permission::OperateSortingDesk);
        return m_desk.activeBatch();
    }

} // namespace hubflow::sorting::application
