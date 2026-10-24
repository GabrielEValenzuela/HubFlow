#include "hubflow/logistics/application/usecases/queries/getOperationsMetrics.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    GetOperationsMetrics::GetOperationsMetrics(const LogisticsModel& model)
        : m_model {model}
    {
    }

    OperationsMetrics GetOperationsMetrics::execute(const shared::application::Principal& principal) const
    {
        principal.require(identity::domain::Permission::OperateShipments);
        OperationsMetrics metrics;
        metrics.totalShipments = m_model.catalog.size();
        m_model.catalog.forEach(
            [&metrics](const domain::Shipment& shipment)
            {
                ++metrics.byStatus[domain::bitOf(shipment.status())];
                metrics.totalAttempts += shipment.attempts();
            });
        m_model.zones.forEach(
            [this, &metrics](const domain::Zone& zone)
            {
                metrics.pendingByZone.pushBack(OperationsMetrics::ZoneEntry {
                    zone.code(),
                    zone.name(),
                    zone.isActive(),
                    domain::summarizeZone(m_model.pending.head(), zone.code(), m_model.priorityPolicy)});
            });
        return metrics;
    }

} // namespace hubflow::logistics::application
