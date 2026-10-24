#include "hubflow/routing/application/usecases/commands/confirmStopParcel.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::application
{

    ConfirmStopParcel::ConfirmStopParcel(logistics::application::LogisticsModel& logistics,
                                         RoutingModel& routing,
                                         shared::application::EventBus& bus,
                                         const shared::application::Clock& clock)
        : m_logistics {logistics}
        , m_routing {routing}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const logistics::domain::Shipment& ConfirmStopParcel::execute(const shared::application::Principal& principal,
                                                                  const ConfirmParcelRequest& request)
    {
        principal.require(identity::domain::Permission::ResolveStops);
        domain::DeliveryRoute& route = m_routing.routes.require(request.routeId);
        if (principal.role == identity::domain::Role::Pum && route.courier().id != principal.userId)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Forbidden, "route.not_owner", "La hoja de ruta pertenece a otro PUM"};
        }
        const auto& stop = route.stop(request.order);
        if (!stop.includes(request.shipmentCode))
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::NotFound,
                                                 "route.shipment_not_in_stop",
                                                 "El paquete no pertenece a esta parada"};
        }
        if (stop.outcome != domain::StopOutcome::Pending)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Conflict, "route.stop_closed", "La parada ya tiene un resultado registrado"};
        }
        logistics::domain::Shipment& shipment = m_logistics.catalog.require(request.shipmentCode);
        shipment.confirmParcel(request.parcelOrdinal, principal.contextAt(m_clock));
        shipment.publishEvents(m_bus);
        return shipment;
    }

} // namespace hubflow::routing::application
