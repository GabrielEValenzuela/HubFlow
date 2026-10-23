#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_RECEPTION_SERVICE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_RECEPTION_SERVICE_HPP

#include "hubflow/logistics/domain/services/deliveryPointCatalog.hpp"
#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"

namespace hubflow::logistics::domain
{

/**
 * @brief Servicio de dominio de recepción de paquetes .
 *
 * Coordina reglas que no pertenecen a un único agregado: unicidad del código en el catálogo,
 * que la zona, el servicio y las características existan y estén habilitados, validez del punto de entrega en el mapa y
 * emisión del ticket de llegada.
 */
class ReceptionService final
{
public:
    ReceptionService(ShipmentCatalog&            catalog,
                     TicketDispenser&            dispenser,
                     const DeliveryPointCatalog& deliveryPoints,
                     const ZoneCatalog&          zones,
                     const ServiceLevelCatalog&  services,
                     const FeatureCatalog&       features);

    /**
     * @brief Recibe el paquete y lo incorpora al catálogo.
     * @throws business_error Conflict (código repetido) o Validation (datos inválidos, zona, servicio o característica
     * inexistente o desactivada, punto inválido).
     */
    Shipment& receive(const ShipmentDraft& draft, const shared::domain::ActorContext& context);

private:
    ShipmentCatalog&            m_catalog;
    TicketDispenser&            m_dispenser;
    const DeliveryPointCatalog& m_deliveryPoints;
    const ZoneCatalog&          m_zones;
    const ServiceLevelCatalog&  m_services;
    const FeatureCatalog&       m_features;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_RECEPTION_SERVICE_HPP
