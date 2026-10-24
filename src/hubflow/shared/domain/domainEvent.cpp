#include "hubflow/shared/domain/domainEvent.hpp"

#include <utility>

namespace hubflow::shared::domain
{

DomainEvent::DomainEvent(ActorContext context)
    : m_context {std::move(context)}
{
}

const ActorContext& DomainEvent::actorContext() const noexcept
{
    return m_context;
}

} // namespace hubflow::shared::domain
