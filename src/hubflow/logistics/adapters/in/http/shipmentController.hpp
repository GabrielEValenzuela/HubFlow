#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_SHIPMENT_CONTROLLER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_SHIPMENT_CONTROLLER_HPP

#include "hubflow/logistics/application/ports/in/findShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/registerShipmentUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::logistics::adapters
{

/// @brief Casos de uso de envíos que expone el controlador.
struct ShipmentUseCases
{
    application::RegisterShipmentUseCase&    registerShipment;
    const application::ListShipmentsUseCase& listShipments;
    const application::FindShipmentUseCase&  findShipment;
    const application::GetShipmentUseCase&   getShipment;
};

/**
 * @brief Alta y consulta de envíos.
 *
 * - @c GET /shipments: catálogo paginado.
 * - @c POST /shipments: recibe un paquete; acepta @c Idempotency-Key, y repetir el mismo código responde 409.
 *   Responde 201 con @c Location.
 * - @c GET /shipments/:code?index=list|bst|bplus: busca un envío con el índice elegido e informa cuántos
 *   elementos visitó.
 * - @c GET /shipments/:code/state-history: estados por los que pasó el envío.
 */
class ShipmentController final : public platform::Controller
{
public:
    explicit ShipmentController(ShipmentUseCases useCases);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    platform::HttpResponse registerShipment(const platform::RequestContext& context) const;
    platform::HttpResponse find(const platform::RequestContext& context) const;

    ShipmentUseCases m_useCases;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_SHIPMENT_CONTROLLER_HPP
