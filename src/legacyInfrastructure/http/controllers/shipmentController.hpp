#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_SHIPMENT_CONTROLLER_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_SHIPMENT_CONTROLLER_HPP

#include "application/logistics/registerShipment.hpp"
#include "application/logistics/shipmentQueries.hpp"
#include "application/logistics/shipmentTransitions.hpp"
#include "infrastructure/http/controllers/controller.hpp"

#include <functional>
#include <map>

namespace hubflow::infrastructure
{

/// @brief Casos de uso de envíos que expone el controlador.
struct ShipmentUseCases
{
    application::RegisterShipment&     registerShipment;
    application::ClassifyShipment&     classify;
    application::DispatchNextShipment& dispatchNext;
    application::RescheduleShipment&   reschedule;
    application::DeliverShipment&      deliver;
    const application::ShipmentQueries& queries;
};

/**
 * @brief Envíos, pendientes, despachos y zonas.
 *
 * - @c GET /shipments, @c POST /shipments (RF01), @c GET /shipments/:code?index=list|bst|bplus
 * - @c GET /shipments/:code/movements?order=asc|desc (RF08), @c POST /shipments/:code/movements (RF04/RF06/RF07)
 * - @c GET /shipments/:code/state-history (RF12)
 * - @c GET /pending-shipments (RF02), @c POST /dispatches (RF05)
 * - @c GET /zones/:zone/summary (recursivo), @c GET /zones/:zone/heaviest-shipment, @c GET /metrics
 */
class ShipmentController final : public Controller
{
public:
    explicit ShipmentController(ShipmentUseCases useCases);

private:
    void registerRoutes(ApiRouter& router) override;

    using Transition = std::function<const domain::logistics::Shipment&(
        const application::Principal&, const std::string&, const std::string&)>;

    HttpResponse recordMovement(const RequestContext& context);

    ShipmentUseCases                                       m_useCases;
    std::map<domain::logistics::ShipmentStatus, Transition> m_transitions;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_SHIPMENT_CONTROLLER_HPP
