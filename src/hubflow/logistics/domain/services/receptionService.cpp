#include "hubflow/logistics/domain/services/receptionService.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

    namespace
    {
        [[noreturn]] void invalid(const char* reason, const char* message)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation, reason, message};
        }
    } // namespace

    ReceptionService::ReceptionService(ShipmentCatalog& catalog,
                                       TicketDispenser& dispenser,
                                       const DeliveryPointCatalog& deliveryPoints,
                                       const ZoneCatalog& zones,
                                       const ServiceLevelCatalog& services,
                                       const FeatureCatalog& features)
        : m_catalog {catalog}
        , m_dispenser {dispenser}
        , m_deliveryPoints {deliveryPoints}
        , m_zones {zones}
        , m_services {services}
        , m_features {features}
    {
    }

    Shipment& ReceptionService::receive(const ShipmentDraft& draft, const shared::domain::ActorContext& context)
    {
        if (m_catalog.contains(draft.code))
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Conflict, "shipment.duplicate", "Ya existe un envío con ese código"};
        }
        const Zone* pZone = m_zones.find(draft.zone);
        if (pZone == nullptr || !pZone->isActive())
        {
            invalid("shipment.zone_unavailable", "La zona no existe o está desactivada");
        }
        const ServiceLevel* pService = m_services.find(draft.service);
        if (pService == nullptr || !pService->isActive())
        {
            invalid("shipment.service_unavailable", "El servicio no existe o está desactivado");
        }
        for (std::size_t featureIndex = 0; featureIndex < draft.features.size(); ++featureIndex)
        {
            const ShipmentFeature* pFeature = m_features.find(draft.features[featureIndex]);
            if (pFeature == nullptr || !pFeature->isActive())
            {
                invalid("shipment.feature_unavailable", "Una característica del paquete no existe o está desactivada");
            }
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
