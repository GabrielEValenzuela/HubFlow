#include "hubflow/routing/application/usecases/commands/resolveRouteStop.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::application
{
    namespace
    {
        constexpr const char* DEFAULT_DELIVERY_NOTE {"Entrega confirmada"};
        constexpr const char* DEFAULT_RESCHEDULE_NOTE {"Destinatario ausente"};

        std::string partialNote(const std::string& note, const logistics::domain::Shipment& shipment)
        {
            const std::size_t delivered {shipment.parcelCount() - shipment.pendingParcelCount()};
            return delivered == 0 ? note
                                  : note + " (entrega parcial: " + std::to_string(delivered) + " de " +
                                        std::to_string(shipment.parcelCount()) + " bultos entregados)";
        }
    } // namespace

    ResolveRouteStop::ResolveRouteStop(logistics::application::LogisticsModel& logistics,
                                       RoutingModel& routing,
                                       shared::application::EventBus& bus,
                                       const shared::application::Clock& clock)
        : m_logistics {logistics}
        , m_routing {routing}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::DeliveryRoute& ResolveRouteStop::execute(const shared::application::Principal& principal,
                                                           const ResolveStopRequest& request)
    {
        principal.require(identity::domain::Permission::ResolveStops);
        if (request.outcome == domain::StopOutcome::Pending)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "route.outcome_invalid",
                                                  "El resultado debe ser ENTREGADO o REPROGRAMADO"};
        }
        domain::DeliveryRoute& route = m_routing.routes.require(request.routeId);
        if (principal.role == identity::domain::Role::Pum && route.courier().id != principal.userId)
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Forbidden, "route.not_owner", "La hoja de ruta pertenece a otro PUM"};
        }
        const auto& stop = route.stop(request.order);
        if (stop.outcome != domain::StopOutcome::Pending && stop.outcome != request.outcome)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "route.stop_closed",
                                                  "La parada ya tiene otro resultado registrado"};
        }
        if (stop.outcome == domain::StopOutcome::Pending)
        {
            const auto context = principal.contextAt(m_clock);
            const bool delivered {request.outcome == domain::StopOutcome::Delivered};
            const std::string fallback {delivered ? DEFAULT_DELIVERY_NOTE : DEFAULT_RESCHEDULE_NOTE};
            const std::string note {request.note.empty() ? fallback : request.note};
            const auto codes = stop.shipmentCodes();
            ds::DynamicArray<logistics::domain::Shipment*> shipments;
            for (std::size_t codeIndex = 0; codeIndex < codes.size(); ++codeIndex)
            {
                logistics::domain::Shipment& shipment = m_logistics.catalog.require(codes[codeIndex]);
                if (delivered && !request.confirmAllParcels && shipment.pendingParcelCount() > 0)
                {
                    throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                          "shipment.parcels_pending",
                                                          "Faltan confirmar bultos del paquete " + shipment.code() +
                                                              ": confirmalos o entregá todo"};
                }
                shipments.pushBack(&shipment);
            }
            for (std::size_t shipmentIndex = 0; shipmentIndex < shipments.size(); ++shipmentIndex)
            {
                if (delivered)
                {
                    shipments[shipmentIndex]->confirmAllParcels(context);
                    shipments[shipmentIndex]->deliver(note, context);
                }
                else
                {
                    shipments[shipmentIndex]->reschedule(
                        partialNote(note, *shipments[shipmentIndex]), m_logistics.tickets.next(), context);
                }
            }
            route.resolveStop(request.order, request.outcome, context);
            for (std::size_t shipmentIndex = 0; shipmentIndex < shipments.size(); ++shipmentIndex)
            {
                shipments[shipmentIndex]->publishEvents(m_bus);
            }
            route.publishEvents(m_bus);
        }
        return route;
    }

} // namespace hubflow::routing::application
