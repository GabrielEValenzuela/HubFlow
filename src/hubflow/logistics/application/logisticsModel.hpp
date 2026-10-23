#ifndef HUBFLOW_LOGISTICS_APPLICATION_LOGISTICS_MODEL_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_LOGISTICS_MODEL_HPP

#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Modelo en memoria del centro de distribución.
 *
 * El catálogo es dueño de los envíos; la lista de pendientes es una proyección mantenida por
 * PendingShipmentsProjector a partir de los eventos de dominio.
 */
struct LogisticsModel
{
    domain::ShipmentCatalog  catalog;
    domain::PendingShipments pending;
    domain::TicketDispenser  tickets;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_LOGISTICS_MODEL_HPP
