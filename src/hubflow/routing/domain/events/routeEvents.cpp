#include "hubflow/routing/domain/events/routeEvents.hpp"

#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <utility>

namespace hubflow::routing::domain
{

DeliveryRouteAssigned::DeliveryRouteAssigned(RouteRecord                  record,
                                             const double                 totalCostKm,
                                             shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_record {std::move(record)}
    , m_totalCostKm {totalCostKm}
{
}

const RouteRecord& DeliveryRouteAssigned::record() const noexcept
{
    return m_record;
}

double DeliveryRouteAssigned::totalCostKm() const noexcept
{
    return m_totalCostKm;
}

void DeliveryRouteAssigned::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

RouteStopResolved::RouteStopResolved(const std::uint64_t          routeId,
                                     RouteStop                    stop,
                                     const bool                   routeCompleted,
                                     shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_routeId {routeId}
    , m_stop {std::move(stop)}
    , m_routeCompleted {routeCompleted}
{
}

std::uint64_t RouteStopResolved::routeId() const noexcept
{
    return m_routeId;
}

const RouteStop& RouteStopResolved::stop() const noexcept
{
    return m_stop;
}

bool RouteStopResolved::routeCompleted() const noexcept
{
    return m_routeCompleted;
}

void RouteStopResolved::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

DeliveryRouteReassigned::DeliveryRouteReassigned(const std::uint64_t          routeId,
                                                 Courier                      from,
                                                 Courier                      to,
                                                 shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_routeId {routeId}
    , m_from {std::move(from)}
    , m_to {std::move(to)}
{
}

std::uint64_t DeliveryRouteReassigned::routeId() const noexcept
{
    return m_routeId;
}

const Courier& DeliveryRouteReassigned::from() const noexcept
{
    return m_from;
}

const Courier& DeliveryRouteReassigned::to() const noexcept
{
    return m_to;
}

void DeliveryRouteReassigned::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

} // namespace hubflow::routing::domain
