#ifndef HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROUTE_PERSISTENCE_SUBSCRIBER_HPP
#define HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROUTE_PERSISTENCE_SUBSCRIBER_HPP

#include "hubflow/routing/application/ports/out/routeRepository.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::routing::application
{

/// @brief Persiste las hojas de ruta y el resultado de cada parada a partir de sus eventos.
class RoutePersistenceSubscriber final : public shared::domain::DomainEventVisitor
{
public:
    explicit RoutePersistenceSubscriber(RouteRepository& repository);

private:
    void on(const domain::DeliveryRouteAssigned& event) override;
    void on(const domain::RouteStopResolved& event) override;

    RouteRepository& m_repository;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROUTE_PERSISTENCE_SUBSCRIBER_HPP
