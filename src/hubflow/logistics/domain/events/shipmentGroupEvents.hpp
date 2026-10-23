#ifndef HUBFLOW_LOGISTICS_DOMAIN_EVENTS_SHIPMENT_GROUP_EVENTS_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_EVENTS_SHIPMENT_GROUP_EVENTS_HPP

#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::logistics::domain
{

/// @brief Base de los eventos de un grupo: el estado del grupo en ese momento.
class ShipmentGroupEvent : public shared::domain::DomainEvent
{
public:
    ShipmentGroupEvent(ShipmentGroupState state, shared::domain::ActorContext context);

    const ShipmentGroupState& state() const noexcept;

private:
    ShipmentGroupState m_state;
};

/// @brief Varios envíos pasaron a viajar juntos.
class ShipmentGroupFormed final : public ShipmentGroupEvent
{
public:
    using ShipmentGroupEvent::ShipmentGroupEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

/// @brief Un grupo se deshizo: sus envíos vuelven a tratarse por separado.
class ShipmentGroupDissolved final : public ShipmentGroupEvent
{
public:
    using ShipmentGroupEvent::ShipmentGroupEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_EVENTS_SHIPMENT_GROUP_EVENTS_HPP
