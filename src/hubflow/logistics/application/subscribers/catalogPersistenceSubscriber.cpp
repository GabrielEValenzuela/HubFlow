#include "hubflow/logistics/application/subscribers/catalogPersistenceSubscriber.hpp"

#include "hubflow/logistics/domain/events/catalogEvents.hpp"

namespace hubflow::logistics::application
{

CatalogPersistenceSubscriber::CatalogPersistenceSubscriber(CatalogRepository& repository)
    : m_repository {repository}
{
}

void CatalogPersistenceSubscriber::on(const domain::ZoneChanged& event)
{
    m_repository.saveZone(event.state());
}

void CatalogPersistenceSubscriber::on(const domain::ServiceLevelChanged& event)
{
    m_repository.saveServiceLevel(event.state());
}

void CatalogPersistenceSubscriber::on(const domain::ShipmentFeatureChanged& event)
{
    m_repository.saveFeature(event.state());
}

} // namespace hubflow::logistics::application
