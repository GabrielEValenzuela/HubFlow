#include "hubflow/logistics/domain/services/receptionService.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

ReceptionService::ReceptionService(ShipmentCatalog&            catalog,
                                   TicketDispenser&            dispenser,
                                   const DeliveryPointCatalog& deliveryPoints)
    : m_catalog {catalog}
    , m_dispenser {dispenser}
    , m_deliveryPoints {deliveryPoints}
{
}

Shipment& ReceptionService::receive(const ShipmentDraft& draft, const shared::domain::ActorContext& context)
{
    if (m_catalog.contains(draft.code))
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Conflict, "shipment.duplicate", "Ya existe un envío con ese código"};
    }
    if (!m_deliveryPoints.isServiceable(draft.deliveryPoint, draft.zone))
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "delivery_point.out_of_zone",
                                              "El punto de entrega no existe en el mapa o no pertenece a la zona"};
    }
    return m_catalog.adopt(Shipment::receive(draft, m_dispenser.next(), context));
}

} // namespace hubflow::logistics::domain
