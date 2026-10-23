#include "hubflow/logistics/application/hydration/logisticsHydrator.hpp"

namespace hubflow::logistics::application
{

LogisticsHydrator::LogisticsHydrator(LogisticsModel& model)
    : m_model {model}
{
}

void LogisticsHydrator::hydrate(const ShipmentRepository& repository)
{
    repository.loadAll(*this);
}

void LogisticsHydrator::restoreShipment(const domain::ShipmentState& state)
{
    domain::Shipment& shipment = m_model.catalog.adopt(domain::Shipment::restore(state));
    m_model.tickets.acknowledge(shipment.ticket());
    if (shipment.isAwaitingDispatch())
    {
        m_model.pending.insert(&shipment);
    }
}

void LogisticsHydrator::restoreMovement(const std::string& code, const domain::Movement& movement)
{
    m_model.catalog.require(code).restoreMovement(movement);
}

} // namespace hubflow::logistics::application
