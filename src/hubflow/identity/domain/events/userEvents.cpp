#include "hubflow/identity/domain/events/userEvents.hpp"

#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <utility>

namespace hubflow::identity::domain
{

    UserEvent::UserEvent(UserState state, shared::domain::ActorContext context)
        : shared::domain::DomainEvent {std::move(context)}
        , m_state {std::move(state)}
    {
    }

    const UserState& UserEvent::state() const noexcept
    {
        return m_state;
    }

    void UserRegistered::accept(shared::domain::DomainEventVisitor& visitor) const
    {
        visitor.on(*this);
    }

    void UserProfileChanged::accept(shared::domain::DomainEventVisitor& visitor) const
    {
        visitor.on(*this);
    }

} // namespace hubflow::identity::domain
