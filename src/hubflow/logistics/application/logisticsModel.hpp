#ifndef HUBFLOW_LOGISTICS_APPLICATION_LOGISTICS_MODEL_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_LOGISTICS_MODEL_HPP

#include "hubflow/logistics/domain/services/catalogPriorityPolicy.hpp"
#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Modelo en memoria del centro de distribución.
 *
 * Reúne los datos maestros (zonas, niveles de servicio y características), el catálogo de envíos y la lista de
 * pendientes. El catálogo es dueño de los envíos; la lista de pendientes es una proyección mantenida por
 * PendingShipmentsProjector a partir de los eventos de dominio y ordenada por la política de prioridad, que por
 * defecto consulta los niveles de servicio del catálogo.
 */
struct LogisticsModel
{
    LogisticsModel()
        : zones {domain::CatalogNouns {"zone", "la zona", "una zona"}}
        , services {domain::CatalogNouns {"service_level", "el servicio", "un servicio"}}
        , features {domain::CatalogNouns {"feature", "la característica", "una característica"}}
        , priorityPolicy {services}
        , pending {priorityPolicy}
    {
    }

    domain::ZoneCatalog           zones;
    domain::ServiceLevelCatalog   services;
    domain::FeatureCatalog        features;
    domain::ShipmentCatalog       catalog;
    domain::CatalogPriorityPolicy priorityPolicy;
    domain::PendingShipments      pending;
    domain::TicketDispenser       tickets;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_LOGISTICS_MODEL_HPP
