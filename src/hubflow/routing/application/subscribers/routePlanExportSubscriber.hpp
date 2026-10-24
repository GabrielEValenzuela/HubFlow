#ifndef HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROUTE_PLAN_EXPORT_SUBSCRIBER_HPP
#define HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROUTE_PLAN_EXPORT_SUBSCRIBER_HPP

#include "hubflow/routing/application/ports/out/routePlanExporter.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::routing::application
{

    /// @brief Exporta el plan de cada hoja de ruta apenas se asigna a un PUM.
    class RoutePlanExportSubscriber final : public shared::domain::DomainEventVisitor
    {
    public:
        RoutePlanExportSubscriber(const RoutingModel& model, RoutePlanExporter& exporter);

    private:
        void on(const domain::DeliveryRouteAssigned& event) override;

        const RoutingModel& m_model;
        RoutePlanExporter& m_exporter;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROUTE_PLAN_EXPORT_SUBSCRIBER_HPP
