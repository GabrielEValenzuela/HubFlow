#include "hubflow/routing/domain/events/roadMapEvents.hpp"

#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <utility>

namespace hubflow::routing::domain
{

RoadNodeAdded::RoadNodeAdded(RoadNode node, ds::DynamicArray<RoadLink> links, shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_node {std::move(node)}
    , m_links {std::move(links)}
{
}

const RoadNode& RoadNodeAdded::node() const noexcept
{
    return m_node;
}

const ds::DynamicArray<RoadLink>& RoadNodeAdded::links() const noexcept
{
    return m_links;
}

void RoadNodeAdded::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

RoadNodeChanged::RoadNodeChanged(RoadNode node, shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_node {std::move(node)}
{
}

const RoadNode& RoadNodeChanged::node() const noexcept
{
    return m_node;
}

void RoadNodeChanged::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

} // namespace hubflow::routing::domain
