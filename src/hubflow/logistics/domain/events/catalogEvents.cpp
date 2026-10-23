#include "hubflow/logistics/domain/events/catalogEvents.hpp"

#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <utility>

namespace hubflow::logistics::domain
{

ZoneChanged::ZoneChanged(ZoneState state, const bool created, shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_state {std::move(state)}
    , m_created {created}
{
}

const ZoneState& ZoneChanged::state() const noexcept
{
    return m_state;
}

bool ZoneChanged::created() const noexcept
{
    return m_created;
}

void ZoneChanged::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

ServiceLevelChanged::ServiceLevelChanged(ServiceLevelState            state,
                                         const bool                   created,
                                         shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_state {std::move(state)}
    , m_created {created}
{
}

const ServiceLevelState& ServiceLevelChanged::state() const noexcept
{
    return m_state;
}

bool ServiceLevelChanged::created() const noexcept
{
    return m_created;
}

void ServiceLevelChanged::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

ShipmentFeatureChanged::ShipmentFeatureChanged(ShipmentFeatureState         state,
                                               const bool                   created,
                                               shared::domain::ActorContext context)
    : shared::domain::DomainEvent {std::move(context)}
    , m_state {std::move(state)}
    , m_created {created}
{
}

const ShipmentFeatureState& ShipmentFeatureChanged::state() const noexcept
{
    return m_state;
}

bool ShipmentFeatureChanged::created() const noexcept
{
    return m_created;
}

void ShipmentFeatureChanged::accept(shared::domain::DomainEventVisitor& visitor) const
{
    visitor.on(*this);
}

} // namespace hubflow::logistics::domain
