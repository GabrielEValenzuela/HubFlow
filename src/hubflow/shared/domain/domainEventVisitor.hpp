#ifndef HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_VISITOR_HPP
#define HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_VISITOR_HPP

namespace hubflow::identity::domain
{
    class UserRegistered;
    class UserProfileChanged;
} // namespace hubflow::identity::domain

namespace hubflow::logistics::domain
{
    class ShipmentRegistered;
    class ShipmentClassified;
    class ShipmentDispatched;
    class ShipmentRescheduled;
    class ShipmentDelivered;
    class ParcelDeliveryConfirmed;
    class ShipmentGroupFormed;
    class ShipmentGroupDissolved;
    class ZoneChanged;
    class ServiceLevelChanged;
    class ShipmentFeatureChanged;
} // namespace hubflow::logistics::domain

namespace hubflow::routing::domain
{
    class DeliveryRouteAssigned;
    class RouteStopResolved;
    class DeliveryRouteReassigned;
    class RoadNodeAdded;
    class RoadNodeChanged;
} // namespace hubflow::routing::domain

namespace hubflow::shared::domain
{

    /**
     * @brief Visitante de eventos de dominio (double dispatch).
     *
     * Cada suscriptor sobrescribe sólo los @c on(...) de los eventos que le interesan; el resto
     * queda como no-op.
     */
    class DomainEventVisitor
    {
    public:
        virtual ~DomainEventVisitor() = default;

        virtual void on(const identity::domain::UserRegistered&) {}

        virtual void on(const identity::domain::UserProfileChanged&) {}

        virtual void on(const logistics::domain::ShipmentRegistered&) {}

        virtual void on(const logistics::domain::ShipmentClassified&) {}

        virtual void on(const logistics::domain::ShipmentDispatched&) {}

        virtual void on(const logistics::domain::ShipmentRescheduled&) {}

        virtual void on(const logistics::domain::ShipmentDelivered&) {}

        virtual void on(const logistics::domain::ParcelDeliveryConfirmed&) {}

        virtual void on(const logistics::domain::ShipmentGroupFormed&) {}

        virtual void on(const logistics::domain::ShipmentGroupDissolved&) {}

        virtual void on(const logistics::domain::ZoneChanged&) {}

        virtual void on(const logistics::domain::ServiceLevelChanged&) {}

        virtual void on(const logistics::domain::ShipmentFeatureChanged&) {}

        virtual void on(const routing::domain::DeliveryRouteAssigned&) {}

        virtual void on(const routing::domain::RouteStopResolved&) {}

        virtual void on(const routing::domain::DeliveryRouteReassigned&) {}

        virtual void on(const routing::domain::RoadNodeAdded&) {}

        virtual void on(const routing::domain::RoadNodeChanged&) {}
    };

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_VISITOR_HPP
