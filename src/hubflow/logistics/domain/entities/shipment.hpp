#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/entities/movementHistory.hpp"
#include "hubflow/logistics/domain/entities/parcel.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogCode.hpp"
#include "hubflow/logistics/domain/valueobjects/deliveryPoint.hpp"
#include "hubflow/logistics/domain/valueobjects/queueTicket.hpp"
#include "hubflow/logistics/domain/valueobjects/recipient.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentId.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <cstdint>
#include <string>

namespace hubflow::logistics::domain
{

/**
 * @brief Datos de alta de un envío. Zona, servicio y características se indican por su código de catálogo.
 *
 * @c id es la identidad interna (UUID versión 7) y @c code el código de seguimiento público; ambos los asigna el
 * sistema al registrar. El envío tiene de 1 a 20 bultos y su peso es el de sus bultos.
 */
struct ShipmentDraft
{
    std::string                   id;
    std::string                   code;
    std::string                   recipient;
    std::string                   zone;
    std::string                   service;
    std::string                   deliveryPoint;
    ds::DynamicArray<std::string> features {};
    ds::DynamicArray<ParcelData>  parcels {};
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
 * @brief Agregado raíz Envío. Identidad: un ShipmentId (UUID versión 7); se lo reconoce afuera por su TrackingCode.
 *
 * Encapsula la máquina de estados: cada comportamiento valida la transición, agrega el
 * movimiento al final de su historial, enciende el bit en su StateHistory y registra el evento
 * de dominio correspondiente. Es dueño de su MovementHistory.
 *
 * Un envío tiene de 1 a 20 bultos. Sólo puede darse por entregado cuando todos sus bultos fueron confirmados; si el
 * repartidor entrega algunos y otros no, el envío se reprograma conservando registrados los bultos ya entregados y
 * cuando vuelve a salir sólo faltan los restantes.
 *
 * La zona, el nivel de servicio y las características son datos de catálogo: el envío sólo guarda sus códigos. Qué
 * tan prioritario es un envío no lo decide el envío sino la política de prioridad de la planta.
 *
 * La copia está prohibida: cada envío existe una sola vez y el resto del sistema lo referencia
 * mediante punteros no propietarios.
 */
class Shipment final : public shared::domain::AggregateRoot
{
public:
    static constexpr std::size_t MAX_PARCELS {20};

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

    /**
     * @brief EN_REPARTO → ENTREGADO (estado final).
     * @throws business_error (Conflict) si queda algún bulto sin confirmar.
     */
    void deliver(const std::string& note, const shared::domain::ActorContext& context);

    /**
     * @brief Confirma que el bulto número @p ordinal (desde 1) llegó a destino. Repetirlo no hace nada.
     * @throws business_error InvalidState (el envío no está en reparto) o NotFound (el bulto no existe).
     */
    void confirmParcel(std::size_t ordinal, const shared::domain::ActorContext& context);

    /// @brief Confirma todos los bultos que faltan.
    void confirmAllParcels(const shared::domain::ActorContext& context);

    /// @brief Restaura un movimiento persistido en el historial.
    void restoreMovement(const Movement& movement);

    ~Shipment() override;

    const std::string& id() const noexcept;
    const std::string& code() const noexcept;
    const std::string& recipient() const noexcept;
    const std::string& zone() const noexcept;

    /// @brief Peso con el que se ordena, resume y factura el envío: la suma del peso facturable de sus bultos.
    double weightKg() const noexcept;

    /// @brief Suma del peso real de los bultos.
    double realWeightKg() const noexcept;

    /// @brief Suma del peso facturable (el mayor entre real y volumétrico) de los bultos.
    double billableWeightKg() const noexcept;

    std::size_t                          parcelCount() const noexcept;
    const Parcel&                        parcel(std::size_t index) const;
    std::size_t                          pendingParcelCount() const noexcept;
    const std::string&                   service() const noexcept;
    const ds::DynamicArray<std::string>& features() const noexcept;
    bool                                 hasFeature(const std::string& code) const noexcept;
    ShipmentStatus                       status() const noexcept;
    std::uint32_t                        attempts() const noexcept;
    const std::string&                   deliveryPoint() const noexcept;
    const StateHistory&                  stateHistory() const noexcept;
    const MovementHistory&               history() const noexcept;
    const QueueTicket&                   ticket() const noexcept;
    bool                                 isAwaitingDispatch() const noexcept;
    ShipmentState                        state() const;

private:
    Shipment(const ShipmentDraft& draft, const QueueTicket& ticket);

    void adoptParcels(const ds::DynamicArray<ParcelData>& parcels);

    const Movement&
    transition(ShipmentStatus target, const std::string& note, const shared::domain::ActorContext& context);

    ShipmentId                    m_id;
    TrackingCode                  m_code;
    Recipient                     m_recipient;
    CatalogCode                   m_zone;
    CatalogCode                   m_service;
    DeliveryPoint                 m_deliveryPoint;
    ds::DynamicArray<std::string> m_features;
    ds::DynamicArray<Parcel*>     m_parcels;
    ShipmentStatus                m_status {ShipmentStatus::Recibido};
    std::uint32_t                 m_attempts {0};
    QueueTicket                   m_ticket;
    StateHistory                  m_stateHistory;
    MovementHistory               m_history;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_HPP
