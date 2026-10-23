#include "hubflow/logistics/domain/events/shipmentEvents.hpp"

#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <utility>

namespace hubflow::logistics::domain
{

ShipmentEvent::ShipmentEvent(ShipmentState state, Movement movement, shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_state {std::move(state)}
    , m_movement {std::move(movement)}
{
}

const ShipmentState& ShipmentEvent::state() const noexcept
{
    return m_state;
}

const Movement& ShipmentEvent::movement() const noexcept
{
    return m_movement;
}

const std::string& ShipmentEvent::code() const noexcept
{
    return m_state.draft.code;
}

void ShipmentRegistered::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

void ShipmentClassified::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

void ShipmentDispatched::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

void ShipmentRescheduled::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

void ShipmentDelivered::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

} // namespace hubflow::logistics::domain
