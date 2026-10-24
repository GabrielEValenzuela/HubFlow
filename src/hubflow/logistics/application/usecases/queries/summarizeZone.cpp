#include "hubflow/logistics/application/usecases/queries/summarizeZone.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    SummarizeZone::SummarizeZone(const LogisticsModel& model)
        : m_model {model}
    {
    }

    domain::ZoneSummary SummarizeZone::execute(const shared::application::Principal& principal,
                                               const std::string& zoneCode) const
    {
        principal.require(identity::domain::Permission::OperateShipments);
        m_model.zones.require(zoneCode);
        return domain::summarizeZone(m_model.pending.head(), zoneCode, m_model.priorityPolicy);
    }

} // namespace hubflow::logistics::application
