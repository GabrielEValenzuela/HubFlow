#include "hubflow/logistics/application/usecases/queries/summarizeZone.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

SummarizeZone::SummarizeZone(const LogisticsModel& model)
    : m_model {model}
{
}

domain::ZoneSummary SummarizeZone::execute(const shared::application::Principal& principal, domain::Zone zone) const
{
    principal.require(identity::domain::Permission::OperateShipments);
    return domain::summarizeZone(m_model.pending.head(), zone);
}

} // namespace hubflow::logistics::application
