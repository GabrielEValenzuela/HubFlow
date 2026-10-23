#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_RECEPTION_SERVICE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_RECEPTION_SERVICE_HPP

#include "hubflow/logistics/domain/services/deliveryPointCatalog.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"

namespace hubflow::logistics::domain
{

/**
 * @brief Servicio de dominio de recepción de paquetes .
 *
 * Coordina reglas que no pertenecen a un único agregado: unicidad del código en el catálogo,
 * validez del punto de entrega en el mapa y emisión del ticket de llegada.
 */
class ReceptionService final
{
public:
    ReceptionService(ShipmentCatalog& catalog, TicketDispenser& dispenser, const DeliveryPointCatalog& deliveryPoints);

    /**
     * @brief Recibe el paquete y lo incorpora al catálogo.
     * @throws business_error Conflict (código repetido) o Validation (datos o punto inválidos).
     */
    Shipment& receive(const ShipmentDraft& draft, const shared::domain::ActorContext& context);

private:
    ShipmentCatalog&            m_catalog;
    TicketDispenser&            m_dispenser;
    const DeliveryPointCatalog& m_deliveryPoints;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_RECEPTION_SERVICE_HPP
