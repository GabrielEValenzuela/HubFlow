#ifndef HUBFLOW_AUDIT_APPLICATION_SUBSCRIBERS_AUDIT_LOG_SUBSCRIBER_HPP
#define HUBFLOW_AUDIT_APPLICATION_SUBSCRIBERS_AUDIT_LOG_SUBSCRIBER_HPP

#include "hubflow/audit/application/ports/out/auditLog.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::audit::application
{

    /// @brief Suscriptor que deja constancia en la auditoría de cada evento de negocio.
    class AuditLogSubscriber final : public shared::domain::DomainEventVisitor
    {
    public:
        explicit AuditLogSubscriber(AuditLog& trail);

    private:
        void on(const identity::domain::UserRegistered& event) override;
        void on(const identity::domain::UserProfileChanged& event) override;
        void on(const logistics::domain::ShipmentRegistered& event) override;
        void on(const logistics::domain::ShipmentClassified& event) override;
        void on(const logistics::domain::ShipmentDispatched& event) override;
        void on(const logistics::domain::ShipmentRescheduled& event) override;
        void on(const logistics::domain::ShipmentDelivered& event) override;
        void on(const logistics::domain::ParcelDeliveryConfirmed& event) override;
        void on(const logistics::domain::ShipmentGroupFormed& event) override;
        void on(const logistics::domain::ShipmentGroupDissolved& event) override;
        void on(const logistics::domain::ZoneChanged& event) override;
        void on(const logistics::domain::ServiceLevelChanged& event) override;
        void on(const logistics::domain::ShipmentFeatureChanged& event) override;
        void on(const routing::domain::DeliveryRouteAssigned& event) override;
        void on(const routing::domain::RouteStopResolved& event) override;
        void on(const routing::domain::DeliveryRouteReassigned& event) override;
        void on(const routing::domain::RoadNodeAdded& event) override;
        void on(const routing::domain::RoadNodeChanged& event) override;

        void write(const shared::domain::DomainEvent& event,
                   const std::string& type,
                   const std::string& subject,
                   const std::string& detail);

        AuditLog& m_trail;
    };

} // namespace hubflow::audit::application

#endif // HUBFLOW_AUDIT_APPLICATION_SUBSCRIBERS_AUDIT_LOG_SUBSCRIBER_HPP
