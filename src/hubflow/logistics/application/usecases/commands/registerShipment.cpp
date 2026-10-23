#include "hubflow/logistics/application/usecases/commands/registerShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/domain/services/receptionService.hpp"

namespace hubflow::logistics::application
{

RegisterShipment::RegisterShipment(LogisticsModel&                     model,
                                   const domain::DeliveryPointCatalog& deliveryPoints,
                                   shared::application::EventBus&      bus,
                                   const shared::application::Clock&   clock)
    : m_model {model}
    , m_deliveryPoints {deliveryPoints}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::Shipment& RegisterShipment::execute(const shared::application::Principal& principal,
                                                  const domain::ShipmentDraft&          draft)
{
    principal.require(identity::domain::Permission::OperateShipments);
    domain::ReceptionService reception {
        m_model.catalog, m_model.tickets, m_deliveryPoints, m_model.zones, m_model.services, m_model.features};
    domain::Shipment& shipment = reception.receive(draft, principal.contextAt(m_clock));
    shipment.publishEvents(m_bus);
    return shipment;
}

} // namespace hubflow::logistics::application
