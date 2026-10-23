#include "hubflow/logistics/domain/entities/shipment.hpp"

#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr const char* RECEPTION_NOTE {"Ingreso al centro de distribución"};
} // namespace

Shipment::Shipment(const ShipmentDraft& draft, const QueueTicket& ticket)
    : m_code {draft.code}
    , m_recipient {draft.recipient}
    , m_zone {draft.zone}
    , m_weight {draft.weightKg}
    , m_service {draft.service}
    , m_deliveryPoint {draft.deliveryPoint}
    , m_ticket {ticket}
{
    for (std::size_t i = 0; i < draft.features.size(); ++i)
    {
        if (!hasFeature(draft.features[i]))
        {
            m_features.pushBack(CatalogCode {draft.features[i]}.value());
        }
    }
}

Shipment*
Shipment::receive(const ShipmentDraft& draft, const QueueTicket ticket, const shared::domain::ActorContext& context)
{
    Shipment* pShipment = new Shipment {draft, ticket};
    pShipment->m_stateHistory.set(bitOf(ShipmentStatus::Recibido));
    const Movement& movement = pShipment->m_history.record(ShipmentStatus::Recibido, RECEPTION_NOTE, context);
    pShipment->raise(new ShipmentRegistered {pShipment->state(), movement, context});
    return pShipment;
}

Shipment* Shipment::restore(const ShipmentState& state)
{
    Shipment* pShipment       = new Shipment {state.draft, QueueTicket {state.ticket}};
    pShipment->m_status       = state.status;
    pShipment->m_attempts     = state.attempts;
    pShipment->m_stateHistory = StateHistory::fromRaw(state.stateBits);
    return pShipment;
}

void Shipment::classify(const std::string& note, const shared::domain::ActorContext& context)
{
    const Movement& movement = transition(ShipmentStatus::Clasificado, note, context);
    raise(new ShipmentClassified {state(), movement, context});
}

void Shipment::dispatch(const std::string& note, const shared::domain::ActorContext& context)
{
    const Movement& movement = transition(ShipmentStatus::EnReparto, note, context);
    raise(new ShipmentDispatched {state(), movement, context});
}

void Shipment::reschedule(const std::string&                  reason,
                          const QueueTicket                   ticket,
                          const shared::domain::ActorContext& context)
{
    const Movement& movement = transition(ShipmentStatus::Reprogramado, reason, context);
    ++m_attempts;
    m_ticket = ticket;
    raise(new ShipmentRescheduled {state(), movement, context});
}

void Shipment::deliver(const std::string& note, const shared::domain::ActorContext& context)
{
    const Movement& movement = transition(ShipmentStatus::Entregado, note, context);
    raise(new ShipmentDelivered {state(), movement, context});
}

void Shipment::restoreMovement(const Movement& movement)
{
    m_history.restore(movement);
}

const Movement&
Shipment::transition(const ShipmentStatus target, const std::string& note, const shared::domain::ActorContext& context)
{
    if (m_status == ShipmentStatus::Entregado)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::InvalidState, "shipment.already_delivered", "El envío ya fue entregado"};
    }
    if (!canTransition(m_status, target))
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::InvalidState,
                                              "shipment.invalid_transition",
                                              "La transición de estado solicitada no está permitida"};
    }
    m_status = target;
    m_stateHistory.set(bitOf(target));
    return m_history.record(target, note, context);
}

const std::string& Shipment::code() const noexcept
{
    return m_code.value();
}

const std::string& Shipment::recipient() const noexcept
{
    return m_recipient.name();
}

const std::string& Shipment::zone() const noexcept
{
    return m_zone.value();
}

double Shipment::weightKg() const noexcept
{
    return m_weight.kilograms();
}

const std::string& Shipment::service() const noexcept
{
    return m_service.value();
}

const ds::DynamicArray<std::string>& Shipment::features() const noexcept
{
    return m_features;
}

bool Shipment::hasFeature(const std::string& code) const noexcept
{
    bool found {false};
    for (std::size_t i = 0; i < m_features.size() && !found; ++i)
    {
        found = m_features[i] == code;
    }
    return found;
}

ShipmentStatus Shipment::status() const noexcept
{
    return m_status;
}

std::uint32_t Shipment::attempts() const noexcept
{
    return m_attempts;
}

const std::string& Shipment::deliveryPoint() const noexcept
{
    return m_deliveryPoint.nodeId();
}

const StateHistory& Shipment::stateHistory() const noexcept
{
    return m_stateHistory;
}

const MovementHistory& Shipment::history() const noexcept
{
    return m_history;
}

const QueueTicket& Shipment::ticket() const noexcept
{
    return m_ticket;
}

bool Shipment::isAwaitingDispatch() const noexcept
{
    return logistics::domain::isAwaitingDispatch(m_status);
}

ShipmentState Shipment::state() const
{
    return ShipmentState {
        ShipmentDraft {code(), recipient(), zone(), weightKg(), service(), deliveryPoint(), m_features},
        m_status,
        m_attempts,
        m_stateHistory.raw(),
        m_ticket.number(),
    };
}

} // namespace hubflow::logistics::domain
