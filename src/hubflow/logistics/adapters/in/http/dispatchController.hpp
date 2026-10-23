#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_DISPATCH_CONTROLLER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_DISPATCH_CONTROLLER_HPP

#include "hubflow/logistics/application/ports/in/dispatchNextShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listPendingShipmentsUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::logistics::adapters
{

/**
 * @brief Cola de envíos pendientes y despacho.
 *
 * - @c GET /pending-shipments: envíos que esperan salir, en el orden en que se despacharán.
 * - @c POST /dispatches {note}: despacha el siguiente envío de la cola. Cada pedido despacha uno distinto,
 *   por eso acepta @c Idempotency-Key para reintentos seguros.
 */
class DispatchController final : public platform::Controller
{
public:
    DispatchController(application::DispatchNextShipmentUseCase&       dispatchNext,
                       const application::ListPendingShipmentsUseCase& listPending);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    application::DispatchNextShipmentUseCase&       m_dispatchNext;
    const application::ListPendingShipmentsUseCase& m_listPending;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_DISPATCH_CONTROLLER_HPP
