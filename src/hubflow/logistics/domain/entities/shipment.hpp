#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_HPP

#include "hubflow/logistics/domain/entities/movementHistory.hpp"
#include "hubflow/logistics/domain/valueobjects/deliveryPoint.hpp"
#include "hubflow/logistics/domain/valueobjects/queueTicket.hpp"
#include "hubflow/logistics/domain/valueobjects/recipient.hpp"
#include "hubflow/logistics/domain/valueobjects/serviceLevel.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/logistics/domain/valueobjects/weight.hpp"
#include "hubflow/logistics/domain/valueobjects/zone.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <cstdint>
#include <string>

namespace hubflow::logistics::domain
{

/// @brief Datos de alta de un envío.
struct ShipmentDraft
{
    std::string  code;
    std::string  recipient;
    Zone         zone {Zone::Centro};
    double       weightKg {0.0};
    ServiceLevel service {ServiceLevel::Estandar};
    std::string  deliveryPoint;
};

/// @brief Estado completo del envío, usado para persistir, restaurar y describir eventos.
struct ShipmentState
{
    ShipmentDraft  draft;
    ShipmentStatus status {ShipmentStatus::Recibido};
    std::uint32_t  attempts {0};
    std::uint32_t  stateBits {0};
    std::uint64_t  ticket {0};
};

/**
 * @brief Agregado raíz Envío. Identidad: su TrackingCode.
 *
 * Encapsula la máquina de estados: cada comportamiento valida la transición, agrega el
 * movimiento al final de su historial, enciende el bit en su StateHistory y registra el evento
 * de dominio correspondiente. Es dueño de su MovementHistory.
 *
 * La copia está prohibida: cada envío existe una sola vez y el resto del sistema lo referencia
 * mediante punteros no propietarios.
 */
class Shipment final : public shared::domain::AggregateRoot
{
public:
    /**
     * @brief Fábrica: recibe un paquete en el centro (estado RECIBIDO) y registra ShipmentRegistered.
     * @return objeto creado con @c new; el llamador debe transferir su ownership al catálogo.
     * @throws business_error (Validation) si algún dato es inválido.
     */
    static Shipment*
    receive(const ShipmentDraft& draft, QueueTicket ticket, const shared::domain::ActorContext& context);

    /// @brief Reconstituye un envío persistido sin generar eventos. Ownership al llamador.
    static Shipment* restore(const ShipmentState& state);

    /// @brief RECIBIDO/REPROGRAMADO → CLASIFICADO.
    void classify(const std::string& note, const shared::domain::ActorContext& context);

    /// @brief Pasa a EN_REPARTO.
    void dispatch(const std::string& note, const shared::domain::ActorContext& context);

    /// @brief EN_REPARTO → REPROGRAMADO, suma un intento y toma un nuevo ticket de llegada.
    void reschedule(const std::string& reason, QueueTicket ticket, const shared::domain::ActorContext& context);

    /// @brief EN_REPARTO → ENTREGADO (estado final).
    void deliver(const std::string& note, const shared::domain::ActorContext& context);

    /// @brief Restaura un movimiento persistido en el historial.
    void restoreMovement(const Movement& movement);

    const std::string&     code() const noexcept;
    const std::string&     recipient() const noexcept;
    Zone                   zone() const noexcept;
    double                 weightKg() const noexcept;
    ServiceLevel           service() const noexcept;
    unsigned               priority() const noexcept;
    ShipmentStatus         status() const noexcept;
    std::uint32_t          attempts() const noexcept;
    const std::string&     deliveryPoint() const noexcept;
    const StateHistory&    stateHistory() const noexcept;
    const MovementHistory& history() const noexcept;
    const QueueTicket&     ticket() const noexcept;
    bool                   isAwaitingDispatch() const noexcept;
    ShipmentState          state() const;

private:
    Shipment(const ShipmentDraft& draft, const QueueTicket& ticket);

    const Movement&
    transition(ShipmentStatus target, const std::string& note, const shared::domain::ActorContext& context);

    TrackingCode    m_code;
    Recipient       m_recipient;
    Zone            m_zone;
    Weight          m_weight;
    ServiceLevel    m_service;
    DeliveryPoint   m_deliveryPoint;
    ShipmentStatus  m_status {ShipmentStatus::Recibido};
    std::uint32_t   m_attempts {0};
    QueueTicket     m_ticket;
    StateHistory    m_stateHistory;
    MovementHistory m_history;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_HPP
