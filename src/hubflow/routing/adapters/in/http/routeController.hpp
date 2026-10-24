#ifndef HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROUTE_CONTROLLER_HPP
#define HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROUTE_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"
#include "hubflow/routing/application/ports/in/assignDeliveryRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/confirmStopParcelUseCase.hpp"
#include "hubflow/routing/application/ports/in/getCourierRouteSheetUseCase.hpp"
#include "hubflow/routing/application/ports/in/getRoadMapUseCase.hpp"
#include "hubflow/routing/application/ports/in/getRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/listRoutesUseCase.hpp"
#include "hubflow/routing/application/ports/in/reassignDeliveryRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/resolveRouteStopUseCase.hpp"

namespace hubflow::routing::adapters
{

    /// @brief Casos de uso de hojas de ruta que expone el controlador.
    struct RouteUseCases
    {
        application::AssignDeliveryRouteUseCase& assign;
        application::ResolveRouteStopUseCase& resolveStop;
        application::ConfirmStopParcelUseCase& confirmParcel;
        application::ReassignDeliveryRouteUseCase& reassign;
        const application::ListRoutesUseCase& list;
        const application::GetRouteUseCase& get;
        const application::GetRoadMapUseCase& roadMap;
        const application::GetCourierRouteSheetUseCase& sheet;
    };

    /**
     * @brief Hojas de ruta de los repartidores (PUM).
     *
     * - @c GET /routes, @c GET /routes/:id: consulta; un PUM sólo ve las propias.
     *   El detalle de la ruta incluye, por parada, el destinatario, el servicio, el peso y la zona del paquete.
     * - @c POST /routes {batchId, courierId, algorithm}: el agente calcula el recorrido del lote y se asigna al
     *   repartidor (acepta @c Idempotency-Key). Responde 201 con @c Location.
     * - @c PUT /routes/:id/courier {courierId}: pasa el reparto en curso a otro chofer activo (jefe de planta o
     *   administrador). Repetir el pedido con el mismo chofer responde 200 sin cambios.
     * - @c PATCH /routes/:id/stops/:order {outcome, note, confirmAllParcels?}: el repartidor informa el resultado de
     * una parada. Dar la parada por entregada exige bultos confirmados o @c confirmAllParcels. Repetir el mismo
     * resultado responde 200 sin cambios; informar otro distinto responde 409.
     * - @c PUT /routes/:id/stops/:order/shipments/:code/parcels/:ordinal: confirma un bulto entregado. Repetirlo
     * responde 200 sin cambios.
     * - @c GET /routes/:id/initial-state y @c GET /routes/:id/sequence: documentos para el simulador, como descarga.
     */
    class RouteController final : public platform::Controller
    {
    public:
        explicit RouteController(RouteUseCases useCases);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        platform::HttpResponse assign(const platform::RequestContext& context) const;
        platform::HttpResponse resolveStop(const platform::RequestContext& context) const;
        platform::HttpResponse confirmParcel(const platform::RequestContext& context) const;
        platform::HttpResponse download(const platform::RequestContext& context, bool initialState) const;

        RouteUseCases m_useCases;
    };

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_IN_HTTP_ROUTE_CONTROLLER_HPP
