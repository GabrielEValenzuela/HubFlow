#include "hubflow/sorting/application/usecases/commands/openSortingBatch.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{

OpenSortingBatch::OpenSortingBatch(const logistics::application::LogisticsModel& model, domain::SortingDesk& desk)
    : m_model {model}
    , m_desk {desk}
{
}

const domain::SortingBatch& OpenSortingBatch::execute(const shared::application::Principal& principal,
                                                      const std::string&                    zone)
{
    principal.require(identity::domain::Permission::OperateSortingDesk);
    m_model.zones.require(zone);
    return m_desk.open(zone, m_model.pending);
}

} // namespace hubflow::sorting::application
