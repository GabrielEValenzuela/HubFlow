#include "hubflow/audit/application/subscribers/auditLogSubscriber.hpp"

#include "hubflow/identity/domain/events/userEvents.hpp"
#include "hubflow/logistics/domain/events/catalogEvents.hpp"
#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/logistics/domain/events/shipmentGroupEvents.hpp"
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

std::string joined(const ds::DynamicArray<std::string>& codes)
{
    std::string text;
    for (std::size_t codeIndex = 0; codeIndex < codes.size(); ++codeIndex)
    {
        text += (codeIndex == 0 ? "" : ", ") + codes[codeIndex];
    }
    return text;
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

void AuditLogSubscriber::on(const logistics::domain::ParcelDeliveryConfirmed& event)
{
    write(event,
          "shipment.parcel_delivered",
          event.code(),
          "Bulto " + std::to_string(event.parcelOrdinal()) + " de " +
              std::to_string(event.state().draft.parcels.size()) + " entregado");
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentGroupFormed& event)
{
    write(event, "shipment_group.formed", event.state().id, "Viajan juntos: " + joined(event.state().codes));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentGroupDissolved& event)
{
    write(
        event, "shipment_group.dissolved", event.state().id, "Dejan de viajar juntos: " + joined(event.state().codes));
}

void AuditLogSubscriber::on(const logistics::domain::ZoneChanged& event)
{
    write(event,
          event.wasCreated() ? "zone.created" : "zone.changed",
          event.state().code,
          event.state().name + " · " + availability(event.state().active));
}

void AuditLogSubscriber::on(const logistics::domain::ServiceLevelChanged& event)
{
    write(event,
          event.wasCreated() ? "service_level.created" : "service_level.changed",
          event.state().code,
          event.state().name + " · prioridad " + std::to_string(event.state().priority) + " · " +
              availability(event.state().active));
}

void AuditLogSubscriber::on(const logistics::domain::ShipmentFeatureChanged& event)
{
    write(event,
          event.wasCreated() ? "feature.created" : "feature.changed",
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
          joined(event.stop().shipmentCodes()) + " · " + shared::application::Vocabulary::name(event.stop().outcome));
}

void AuditLogSubscriber::on(const routing::domain::DeliveryRouteReassigned& event)
{
    write(event,
          "route.reassigned",
          "Ruta #" + std::to_string(event.routeId()),
          event.previousCourier().name + " → " + event.newCourier().name);
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
    const auto& context = event.actorContext();
    m_trail.append(AuditEntry {0, context.occurredAt, context.userId, context.actorName, type, subject, detail});
}

} // namespace hubflow::audit::application
