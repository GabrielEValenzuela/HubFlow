#include "hubflow/routing/application/subscribers/routePlanExportSubscriber.hpp"

#include "hubflow/routing/domain/events/routeEvents.hpp"

namespace hubflow::routing::application
{

RoutePlanExportSubscriber::RoutePlanExportSubscriber(const RoutingModel& model, RoutePlanExporter& exporter)
    : m_model {model}
    , m_exporter {exporter}
{
}

void RoutePlanExportSubscriber::on(const domain::DeliveryRouteAssigned& event)
{
    m_exporter.exportPlan(m_model.map, m_model.routes.require(event.record().id));
}

} // namespace hubflow::routing::application
