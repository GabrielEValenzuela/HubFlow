#ifndef HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_PENDING_SHIPMENTS_PROJECTOR_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_PENDING_SHIPMENTS_PROJECTOR_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Mantiene la lista de pendientes consistente con los eventos de envío.
 *
 * - ShipmentRegistered / ShipmentRescheduled → inserta por prioridad (estable).
 * - ShipmentDispatched → quita el nodo (el envío sigue vivo en el catálogo).
 * - ShipmentDelivered → nada: ya no estaba en pendientes y nunca vuelve.
 */
class PendingShipmentsProjector final : public shared::domain::DomainEventVisitor
{
public:
    explicit PendingShipmentsProjector(LogisticsModel& model);

private:
    void on(const domain::ShipmentRegistered& event) override;
    void on(const domain::ShipmentDispatched& event) override;
    void on(const domain::ShipmentRescheduled& event) override;

    LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_PENDING_SHIPMENTS_PROJECTOR_HPP
