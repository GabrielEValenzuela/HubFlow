#include "hubflow/routing/application/subscribers/routePersistenceSubscriber.hpp"

#include "hubflow/routing/domain/events/routeEvents.hpp"

namespace hubflow::routing::application
{

RoutePersistenceSubscriber::RoutePersistenceSubscriber(RouteRepository& repository)
    : m_repository {repository}
{
}

void RoutePersistenceSubscriber::on(const domain::DeliveryRouteAssigned& event)
{
    m_repository.add(event.record());
}

void RoutePersistenceSubscriber::on(const domain::RouteStopResolved& event)
{
    m_repository.updateStop(event.routeId(), event.stop());
}

void RoutePersistenceSubscriber::on(const domain::DeliveryRouteReassigned& event)
{
    m_repository.updateCourier(event.routeId(), event.newCourier());
}

} // namespace hubflow::routing::application
