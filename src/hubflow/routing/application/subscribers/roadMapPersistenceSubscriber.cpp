#include "hubflow/routing/application/subscribers/roadMapPersistenceSubscriber.hpp"

#include "hubflow/routing/domain/events/roadMapEvents.hpp"

namespace hubflow::routing::application
{

RoadMapPersistenceSubscriber::RoadMapPersistenceSubscriber(RoadMapRepository& repository)
    : m_repository {repository}
{
}

void RoadMapPersistenceSubscriber::on(const domain::RoadNodeAdded& event)
{
    m_repository.addNode(event.node(), event.links());
}

void RoadMapPersistenceSubscriber::on(const domain::RoadNodeChanged& event)
{
    m_repository.updateNode(event.node());
}

} // namespace hubflow::routing::application
