#include "hubflow/logistics/domain/events/shipmentGroupEvents.hpp"

#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <utility>

namespace hubflow::logistics::domain
{

    ShipmentGroupEvent::ShipmentGroupEvent(ShipmentGroupState state, shared::domain::ActorContext context)
        : shared::domain::DomainEvent {std::move(context)}
        , m_state {std::move(state)}
    {
    }

    const ShipmentGroupState& ShipmentGroupEvent::state() const noexcept
    {
        return m_state;
    }

    void ShipmentGroupFormed::accept(shared::domain::DomainEventVisitor& visitor) const
    {
        visitor.on(*this);
    }

    void ShipmentGroupDissolved::accept(shared::domain::DomainEventVisitor& visitor) const
    {
        visitor.on(*this);
    }

} // namespace hubflow::logistics::domain
