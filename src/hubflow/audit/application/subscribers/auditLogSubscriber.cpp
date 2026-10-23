#include "hubflow/audit/application/subscribers/auditLogSubscriber.hpp"

#include "hubflow/identity/domain/events/userEvents.hpp"
#include "hubflow/logistics/domain/events/catalogEvents.hpp"
#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/routing/domain/events/roadMapEvents.hpp"
#include "hubflow/routing/domain/events/routeEvents.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::audit::application
{
namespace
{
std::string shipmentDetail(const logistics::domain::ShipmentEvent& event)
{
    return shared::application::Vocabulary::name(event.state().status) + " · " + event.movement().note;
}

std::string availability(const bool active)
{
    return active ? "habilitado" : "desactivado";
}
} // namespace

AuditLogSubscriber::AuditLogSubscriber(AuditLog& trail)
    : m_trail {trail}
{
}

void AuditLogSubscriber::on(const identity::domain::UserRegistered& event)
{
    write(event, "user.registered", event.state().email, shared::application::Vocabulary::name(event.state().role));
}

void AuditLogSubscriber::on(const identity::domain::UserProfileChanged& event)
{
    const std::string status {event.state().active ? "activo" : "inactivo"};
    write(event,
          "user.changed",
          event.state().email,
          shared::application::Vocabulary::name(event.state().role) + " · " + status);
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentRegistered& event)
{
    write(event, "shipment.registered", event.code(), shipmentDetail(event));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentClassified& event)
{
    write(event, "shipment.classified", event.code(), shipmentDetail(event));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentDispatched& event)
{
    write(event, "shipment.dispatched", event.code(), shipmentDetail(event));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentRescheduled& event)
{
    write(event, "shipment.rescheduled", event.code(), shipmentDetail(event));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentDelivered& event)
{
    write(event, "shipment.delivered", event.code(), shipmentDetail(event));
}

void AuditLogSubscriber::on(const logistics::domain::ZoneChanged& event)
{
    write(event,
          event.created() ? "zone.created" : "zone.changed",
          event.state().code,
          event.state().name + " · " + availability(event.state().active));
}

void AuditLogSubscriber::on(const logistics::domain::ServiceLevelChanged& event)
{
    write(event,
          event.created() ? "service_level.created" : "service_level.changed",
          event.state().code,
          event.state().name + " · prioridad " + std::to_string(event.state().priority) + " · " +
              availability(event.state().active));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentFeatureChanged& event)
{
    write(event,
          event.created() ? "feature.created" : "feature.changed",
          event.state().code,
          event.state().name + " · " + availability(event.state().active));
}

void AuditLogSubscriber::on(const routing::domain::DeliveryRouteAssigned& event)
{
    const auto& record = event.record();
    write(event,
          "route.assigned",
          "Ruta #" + std::to_string(record.id),
          record.courier.name + " · " + shared::application::Vocabulary::name(record.algorithm) + " · " +
              std::to_string(record.stops.size()) + " paradas");
}

void AuditLogSubscriber::on(const routing::domain::RouteStopResolved& event)
{
    write(event,
          "route.stop_resolved",
          "Ruta #" + std::to_string(event.routeId()),
          event.stop().shipmentCode + " · " + shared::application::Vocabulary::name(event.stop().outcome));
}

void AuditLogSubscriber::on(const routing::domain::DeliveryRouteReassigned& event)
{
    write(event,
          "route.reassigned",
          "Ruta #" + std::to_string(event.routeId()),
          event.from().name + " → " + event.to().name);
}

void AuditLogSubscriber::on(const routing::domain::RoadNodeAdded& event)
{
    write(event,
          "road_node.added",
          event.node().id,
          event.node().label + " · zona " + event.node().zone + " · " + std::to_string(event.links().size()) +
              " calles");
}

void AuditLogSubscriber::on(const routing::domain::RoadNodeChanged& event)
{
    write(event,
          "road_node.changed",
          event.node().id,
          "zona " + event.node().zone + " · " + availability(event.node().active));
}

void AuditLogSubscriber::write(const shared::domain::DomainEvent& event,
                               const std::string&                 type,
                               const std::string&                 subject,
                               const std::string&                 detail)
{
    const auto& context = event.context();
    m_trail.append(AuditEntry {0, context.occurredAt, context.userId, context.actorName, type, subject, detail});
}

} // namespace hubflow::audit::application
