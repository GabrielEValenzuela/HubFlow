#ifndef HUBFLOW_ROUTING_DOMAIN_EVENTS_ROUTE_EVENTS_HPP
#define HUBFLOW_ROUTING_DOMAIN_EVENTS_ROUTE_EVENTS_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::routing::domain
{

/// @brief Se asignó una hoja de ruta a un PUM.
class DeliveryRouteAssigned final : public shared::domain::DomainEvent
{
public:
    DeliveryRouteAssigned(RouteRecord record, double totalCostKm, shared::domain::ActorContext context);

    const RouteRecord& record() const noexcept;
    double             totalCostKm() const noexcept;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;

    RouteRecord m_record;
    double      m_totalCostKm;
};

/// @brief El PUM informó el resultado de una parada.
class RouteStopResolved final : public shared::domain::DomainEvent
{
public:
    RouteStopResolved(std::uint64_t routeId, RouteStop stop, bool routeCompleted, shared::domain::ActorContext context);

    std::uint64_t    routeId() const noexcept;
    const RouteStop& stop() const noexcept;
    bool             routeCompleted() const noexcept;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;

    std::uint64_t m_routeId;
    RouteStop     m_stop;
    bool          m_routeCompleted;
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_EVENTS_ROUTE_EVENTS_HPP
