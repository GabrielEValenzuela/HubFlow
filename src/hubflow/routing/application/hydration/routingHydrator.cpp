#include "hubflow/routing/application/hydration/routingHydrator.hpp"

namespace hubflow::routing::application
{

RoutingHydrator::RoutingHydrator(RoutingModel& model)
    : m_model {model}
{
}

void RoutingHydrator::hydrate(const RoadMapRepository& maps, const RouteRepository& routes)
{
    maps.loadInto(m_model.map);
    routes.loadAll(*this);
}

void RoutingHydrator::restoreRoute(const domain::RouteRecord& record)
{
    const domain::PumAgent agent {m_model.map, domain::SearchStrategy::of(record.algorithm)};
    m_model.routes.adopt(domain::DeliveryRoute::restore(record, agent));
}

} // namespace hubflow::routing::application
