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
        : m_id {draft.id}
        , m_code {draft.code}
        , m_recipient {draft.recipient, draft.recipientPhone, draft.recipientPostalCode}
        , m_zone {draft.zone}
        , m_service {draft.service}
        , m_deliveryPoint {draft.deliveryPoint}
        , m_ticket {ticket}
    {
        for (std::size_t featureIndex = 0; featureIndex < draft.features.size(); ++featureIndex)
        {
            if (!hasFeature(draft.features[featureIndex]))
            {
                m_features.pushBack(CatalogCode {draft.features[featureIndex]}.value());
            }
        }
        adoptParcels(draft.parcels);
    }

    Shipment::~Shipment()
    {
        for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
        {
            delete m_parcels[parcelIndex];
        }
    }

    void Shipment::adoptParcels(const ds::DynamicArray<ParcelData>& parcels)
    {
        if (parcels.isEmpty() || parcels.size() > MAX_PARCELS)
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::Validation,
                                                 "shipment.parcels_invalid",
                                                 "El envío debe tener entre 1 y 20 bultos"};
        }
        try
        {
            for (std::size_t parcelIndex = 0; parcelIndex < parcels.size(); ++parcelIndex)
            {
                m_parcels.pushBack(new Parcel {m_code.value(), parcelIndex + 1, parcels.size(), parcels[parcelIndex]});
            }
        }
        catch (...)
        {
            for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
            {
                delete m_parcels[parcelIndex];
            }
            m_parcels.clear();
            throw;
        }
    }

    Shipment*
    Shipment::receive(const ShipmentDraft& draft, const QueueTicket ticket, const shared::domain::ActorContext& context)
    {
        auto* pShipment = new Shipment {draft, ticket};
        pShipment->m_stateHistory.set(bitOf(ShipmentStatus::Recibido));
        const Movement& movement = pShipment->m_history.record(ShipmentStatus::Recibido, RECEPTION_NOTE, context);
        pShipment->raise(new ShipmentRegistered {pShipment->state(), movement, context});
        return pShipment;
    }

    Shipment* Shipment::restore(const ShipmentState& state)
    {
        auto* pShipment = new Shipment {state.draft, QueueTicket {state.ticket}};
        pShipment->m_status = state.status;
        pShipment->m_attempts = state.attempts;
        pShipment->m_stateHistory = StateHistory::fromWord(state.stateBits);
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

    void Shipment::reschedule(const std::string& reason,
                              const QueueTicket queueTicket,
                              const shared::domain::ActorContext& context)
    {
        const Movement& movement = transition(ShipmentStatus::Reprogramado, reason, context);
        ++m_attempts;
        m_ticket = queueTicket;
        raise(new ShipmentRescheduled {state(), movement, context});
    }

    void Shipment::deliver(const std::string& note, const shared::domain::ActorContext& context)
    {
        if (m_status == ShipmentStatus::EnReparto && pendingParcelCount() > 0)
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::Conflict,
                                                 "shipment.parcels_pending",
                                                 "Faltan confirmar " + std::to_string(pendingParcelCount()) +
                                                     " bultos antes de dar el paquete por entregado"};
        }
        const Movement& movement = transition(ShipmentStatus::Entregado, note, context);
        raise(new ShipmentDelivered {state(), movement, context});
    }

    void Shipment::confirmParcel(const std::size_t ordinal, const shared::domain::ActorContext& context)
    {
        if (m_status != ShipmentStatus::EnReparto)
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::InvalidState,
                                                 "shipment.not_in_delivery",
                                                 "Sólo se pueden confirmar bultos de un paquete que está en reparto"};
        }
        if (ordinal == 0 || ordinal > m_parcels.size())
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::NotFound, "parcel.not_found", "El bulto no existe en este paquete"};
        }
        Parcel* pParcel = m_parcels[ordinal - 1];
        if (!pParcel->isDelivered())
        {
            pParcel->confirmDelivery();
            raise(new ParcelDeliveryConfirmed {state(), ordinal, context});
        }
    }

    void Shipment::confirmAllParcels(const shared::domain::ActorContext& context)
    {
        for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
        {
            confirmParcel(parcelIndex + 1, context);
        }
    }

    void Shipment::restoreMovement(const Movement& movement)
    {
        m_history.restore(movement);
    }

    const Movement& Shipment::transition(const ShipmentStatus target,
                                         const std::string& note,
                                         const shared::domain::ActorContext& context)
    {
        if (m_status == ShipmentStatus::Entregado)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::InvalidState, "shipment.already_delivered", "El envío ya fue entregado"};
        }
        if (!canTransition(m_status, target))
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::InvalidState,
                                                 "shipment.invalid_transition",
                                                 "La transición de estado solicitada no está permitida"};
        }
        m_status = target;
        m_stateHistory.set(bitOf(target));
        return m_history.record(target, note, context);
    }

    const std::string& Shipment::id() const noexcept
    {
        return m_id.value();
    }

    const std::string& Shipment::code() const noexcept
    {
        return m_code.value();
    }

    const std::string& Shipment::recipient() const noexcept
    {
        return m_recipient.name();
    }

    const Recipient& Shipment::contact() const noexcept
    {
        return m_recipient;
    }

    const std::string& Shipment::zone() const noexcept
    {
        return m_zone.value();
    }

    double Shipment::weightKg() const noexcept
    {
        return billableWeightKg();
    }

    double Shipment::realWeightKg() const noexcept
    {
        double total {0.0};
        for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
        {
            total += m_parcels[parcelIndex]->weightKg();
        }
        return total;
    }

    double Shipment::billableWeightKg() const noexcept
    {
        double total {0.0};
        for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
        {
            total += m_parcels[parcelIndex]->billableWeightKg();
        }
        return total;
    }

    std::size_t Shipment::parcelCount() const noexcept
    {
        return m_parcels.size();
    }

    const Parcel& Shipment::parcel(const std::size_t index) const
    {
        return *m_parcels[index];
    }

    std::size_t Shipment::pendingParcelCount() const noexcept
    {
        std::size_t pending {0};
        for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
        {
            pending += m_parcels[parcelIndex]->isDelivered() ? 0 : 1;
        }
        return pending;
    }

    const std::string& Shipment::service() const noexcept
    {
        return m_service.value();
    }

    const ds::DynamicArray<std::string>& Shipment::features() const noexcept
    {
        return m_features;
    }

    bool Shipment::hasFeature(const std::string& featureCode) const noexcept
    {
        bool found {false};
        for (std::size_t i = 0; i < m_features.size() && !found; ++i)
        {
            found = m_features[i] == featureCode;
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
        ds::DynamicArray<ParcelData> parcels;
        for (std::size_t parcelIndex = 0; parcelIndex < m_parcels.size(); ++parcelIndex)
        {
            parcels.pushBack(m_parcels[parcelIndex]->details());
        }
        return ShipmentState {
            ShipmentDraft {id(),
                           code(),
                           recipient(),
                           m_recipient.phone().digits(),
                           m_recipient.postalCode().value(),
                           zone(),
                           service(),
                           deliveryPoint(),
                           m_features,
                           parcels},
            m_status,
            m_attempts,
            m_stateHistory.toWord(),
            m_ticket.number(),
        };
    }

} // namespace hubflow::logistics::domain
