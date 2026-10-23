#include "hubflow/logistics/application/hydration/logisticsHydrator.hpp"

namespace hubflow::logistics::application
{

LogisticsHydrator::LogisticsHydrator(LogisticsModel& model)
    : m_model {model}
{
}

void LogisticsHydrator::hydrate(const CatalogRepository&       catalogs,
                                const ShipmentRepository&      shipments,
                                const ShipmentGroupRepository& groups)
{
    catalogs.loadAll(*this);
    shipments.loadAll(*this);
    groups.loadAll(*this);
}

void LogisticsHydrator::restoreGroup(const domain::ShipmentGroupState& state)
{
    m_model.groups.adopt(domain::ShipmentGroup::restore(state));
}

void LogisticsHydrator::restoreZone(const domain::ZoneState& state)
{
    m_model.zones.adopt(domain::Zone::restore(state));
}

void LogisticsHydrator::restoreServiceLevel(const domain::ServiceLevelState& state)
{
    m_model.services.adopt(domain::ServiceLevel::restore(state));
}

void LogisticsHydrator::restoreFeature(const domain::ShipmentFeatureState& state)
{
    m_model.features.adopt(domain::ShipmentFeature::restore(state));
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
