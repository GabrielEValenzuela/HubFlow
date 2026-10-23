#ifndef HUBFLOW_LOGISTICS_DOMAIN_EVENTS_SHIPMENT_EVENTS_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_EVENTS_SHIPMENT_EVENTS_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::logistics::domain
{

/// @brief Base de los eventos del envío: estado resultante y movimiento generado.
class ShipmentEvent : public shared::domain::DomainEvent
{
public:
    ShipmentEvent(ShipmentState state, Movement movement, shared::domain::ActorContext context);

    const ShipmentState& state() const noexcept;
    const Movement&      movement() const noexcept;
    const std::string&   code() const noexcept;

private:
    ShipmentState m_state;
    Movement      m_movement;
};

/// @brief Un paquete ingresó al centro de distribución.
class ShipmentRegistered final : public ShipmentEvent
{
public:
    using ShipmentEvent::ShipmentEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

/// @brief El envío fue clasificado y sigue esperando despacho.
class ShipmentClassified final : public ShipmentEvent
{
public:
    using ShipmentEvent::ShipmentEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

/// @brief El envío salió a reparto (abandona la lista de pendientes).
class ShipmentDispatched final : public ShipmentEvent
{
public:
    using ShipmentEvent::ShipmentEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

/// @brief La entrega falló: vuelve a pendientes respetando su prioridad.
class ShipmentRescheduled final : public ShipmentEvent
{
public:
    using ShipmentEvent::ShipmentEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

/// @brief El envío fue entregado (estado terminal).
class ShipmentDelivered final : public ShipmentEvent
{
public:
    using ShipmentEvent::ShipmentEvent;

private:
    void accept(shared::domain::DomainEventVisitor& visitor) const override;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_EVENTS_SHIPMENT_EVENTS_HPP
