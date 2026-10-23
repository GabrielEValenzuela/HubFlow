#ifndef HUBFLOW_LOGISTICS_DOMAIN_EVENTS_CATALOG_EVENTS_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_EVENTS_CATALOG_EVENTS_HPP

#include "hubflow/logistics/domain/entities/serviceLevel.hpp"
#include "hubflow/logistics/domain/entities/shipmentFeature.hpp"
#include "hubflow/logistics/domain/entities/zone.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::logistics::domain
{

/// @brief Se dio de alta o cambió una zona (nombre o habilitación). Lleva el estado resultante.
class ZoneChanged final : public shared::domain::DomainEvent
{
public:
    ZoneChanged(ZoneState state, bool created, shared::domain::ActorContext context);

    const ZoneState& state() const noexcept;
    bool             created() const noexcept;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;

    ZoneState m_state;
    bool      m_created;
};

/// @brief Se dio de alta o cambió un nivel de servicio. Lleva el estado resultante.
class ServiceLevelChanged final : public shared::domain::DomainEvent
{
public:
    ServiceLevelChanged(ServiceLevelState state, bool created, shared::domain::ActorContext context);

    const ServiceLevelState& state() const noexcept;
    bool                     created() const noexcept;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;

    ServiceLevelState m_state;
    bool              m_created;
};

/// @brief Se dio de alta o cambió una característica de envío. Lleva el estado resultante.
class ShipmentFeatureChanged final : public shared::domain::DomainEvent
{
public:
    ShipmentFeatureChanged(ShipmentFeatureState state, bool created, shared::domain::ActorContext context);

    const ShipmentFeatureState& state() const noexcept;
    bool                        created() const noexcept;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;

    ShipmentFeatureState m_state;
    bool                 m_created;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_EVENTS_CATALOG_EVENTS_HPP
