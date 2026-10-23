#ifndef HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROAD_MAP_PERSISTENCE_SUBSCRIBER_HPP
#define HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROAD_MAP_PERSISTENCE_SUBSCRIBER_HPP

#include "hubflow/routing/application/ports/out/roadMapRepository.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::routing::application
{

/// @brief Persiste los puntos nuevos del mapa y los cambios de zona o habilitación a partir de sus eventos.
class RoadMapPersistenceSubscriber final : public shared::domain::DomainEventVisitor
{
public:
    explicit RoadMapPersistenceSubscriber(RoadMapRepository& repository);

private:
    void on(const domain::RoadNodeAdded& event) override;
    void on(const domain::RoadNodeChanged& event) override;

    RoadMapRepository& m_repository;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_SUBSCRIBERS_ROAD_MAP_PERSISTENCE_SUBSCRIBER_HPP
