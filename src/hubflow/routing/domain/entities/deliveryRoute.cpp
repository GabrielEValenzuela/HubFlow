#include "hubflow/routing/domain/entities/deliveryRoute.hpp"

#include "hubflow/routing/domain/events/routeEvents.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::domain
{

DeliveryRoute::DeliveryRoute(const RouteRecord& record)
    : m_id {record.id}
    , m_courier {record.courier}
    , m_algorithm {record.algorithm}
    , m_createdAt {record.createdAt}
    , m_stops {record.stops}
{
}

DeliveryRoute::~DeliveryRoute()
{
    m_segments.forEach(
        [](SearchTrace* pTrace)
        {
            delete pTrace;
        });
}

DeliveryRoute* DeliveryRoute::assign(const std::uint64_t                 id,
                                     const Courier&                      courier,
                                     const ds::DynamicArray<RouteStop>&  stops,
                                     const PumAgent&                     agent,
                                     const shared::domain::ActorContext& context)
{
    if (stops.empty())
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Conflict, "route.no_stops", "La ruta necesita al menos una parada"};
    }
    DeliveryRoute* pRoute = new DeliveryRoute {RouteRecord {id, courier, agent.algorithm(), context.occurredAt, stops}};
    try
    {
        pRoute->plan(agent);
    }
    catch (...)
    {
        delete pRoute;
        throw;
    }
    pRoute->raise(new DeliveryRouteAssigned {pRoute->record(), pRoute->totalCostKm(), context});
    return pRoute;
}

DeliveryRoute* DeliveryRoute::restore(const RouteRecord& record, const PumAgent& agent)
{
    DeliveryRoute* pRoute = new DeliveryRoute {record};
    try
    {
        pRoute->plan(agent);
    }
    catch (...)
    {
        delete pRoute;
        throw;
    }
    return pRoute;
}

void DeliveryRoute::plan(const PumAgent& agent)
{
    const RoadMap&    map = agent.map();
    const std::size_t hub {map.hub()};
    std::size_t       current {hub};
    for (std::size_t i = 0; i <= m_stops.size(); ++i)
    {
        const std::size_t target {i < m_stops.size() ? map.require(m_stops[i].nodeId) : hub};
        SearchTrace*      pTrace = new SearchTrace {agent.travel(current, target)};
        m_segments.pushBack(pTrace);
        if (!pTrace->metrics.solutionFound)
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::InvalidState, "route.unreachable", "El PUM no encontró camino a una parada"};
        }
        current = target;
    }
}

const RouteStop& DeliveryRoute::resolveStop(const std::size_t                   order,
                                            const StopOutcome                   outcome,
                                            const shared::domain::ActorContext& context)
{
    if (outcome == StopOutcome::Pending)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "route.outcome_invalid",
                                              "El resultado de una parada no puede ser Pendiente"};
    }
    RouteStop& target = m_stops[stop(order).order - 1];
    if (target.outcome == outcome)
    {
        return target;
    }
    if (target.outcome != StopOutcome::Pending)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Conflict, "route.stop_closed", "La parada ya tiene otro resultado registrado"};
    }
    target.outcome = outcome;
    raise(new RouteStopResolved {m_id, target, status() == RouteStatus::Completed, context});
    return target;
}

void DeliveryRoute::reassign(const Courier& courier, const shared::domain::ActorContext& context)
{
    if (courier.id != m_courier.id)
    {
        if (status() == RouteStatus::Completed)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "route.completed",
                                                  "El reparto ya terminó: no se puede pasar a otro chofer"};
        }
        const Courier previous {m_courier};
        m_courier = courier;
        raise(new DeliveryRouteReassigned {m_id, previous, m_courier, context});
    }
}

const RouteStop& DeliveryRoute::stop(const std::size_t order) const
{
    if (order == 0 || order > m_stops.size())
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::NotFound, "route.stop_not_found", "Parada inexistente"};
    }
    return m_stops[order - 1];
}

std::uint64_t DeliveryRoute::id() const noexcept
{
    return m_id;
}

const Courier& DeliveryRoute::courier() const noexcept
{
    return m_courier;
}

SearchAlgorithm DeliveryRoute::algorithm() const noexcept
{
    return m_algorithm;
}

std::int64_t DeliveryRoute::createdAt() const noexcept
{
    return m_createdAt;
}

const ds::DynamicArray<RouteStop>& DeliveryRoute::stops() const noexcept
{
    return m_stops;
}

RouteStatus DeliveryRoute::status() const noexcept
{
    bool pending {false};
    for (std::size_t i = 0; i < m_stops.size() && !pending; ++i)
    {
        pending = m_stops[i].outcome == StopOutcome::Pending;
    }
    return pending ? RouteStatus::InProgress : RouteStatus::Completed;
}

double DeliveryRoute::totalCostKm() const noexcept
{
    double total {0.0};
    m_segments.forEach(
        [&total](const SearchTrace* pTrace)
        {
            total += pTrace->metrics.costTotal;
        });
    return total;
}

std::size_t DeliveryRoute::totalNodesExplored() const noexcept
{
    std::size_t total {0};
    m_segments.forEach(
        [&total](const SearchTrace* pTrace)
        {
            total += pTrace->metrics.nodesExplored;
        });
    return total;
}

RouteRecord DeliveryRoute::record() const
{
    return RouteRecord {m_id, m_courier, m_algorithm, m_createdAt, m_stops};
}

} // namespace hubflow::routing::domain
