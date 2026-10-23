#include "hubflow/logistics/application/subscribers/pendingShipmentsProjector.hpp"

#include "hubflow/logistics/domain/events/shipmentEvents.hpp"

namespace hubflow::logistics::application
{

PendingShipmentsProjector::PendingShipmentsProjector(LogisticsModel& model)
    : m_model {model}
{
}

void PendingShipmentsProjector::on(const domain::ShipmentRegistered& event)
{
    m_model.pending.insert(&m_model.catalog.require(event.code()));
}

void PendingShipmentsProjector::on(const domain::ShipmentDispatched& event)
{
    m_model.pending.remove(&m_model.catalog.require(event.code()));
}

void PendingShipmentsProjector::on(const domain::ShipmentRescheduled& event)
{
    m_model.pending.insert(&m_model.catalog.require(event.code()));
}

} // namespace hubflow::logistics::application
