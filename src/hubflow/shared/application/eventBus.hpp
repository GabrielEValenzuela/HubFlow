#ifndef HUBFLOW_SHARED_APPLICATION_EVENT_BUS_HPP
#define HUBFLOW_SHARED_APPLICATION_EVENT_BUS_HPP

#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/shared/domain/domainEventPublisher.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::shared::application
{

/**
 * @brief Bus de eventos síncrono en proceso.
 *
 * Entrega cada evento a los suscriptores en el orden en que se suscribieron: primero las
 * proyecciones del modelo en memoria, luego la persistencia y por último la auditoría. Guarda
 * punteros no propietarios a los suscriptores.
 */
class EventBus final : public domain::DomainEventPublisher
{
public:
    /// @brief Agrega un suscriptor; debe vivir al menos tanto como el bus.
    void subscribe(domain::DomainEventVisitor& subscriber);

private:
    void publish(const domain::DomainEvent& event) override;

    ds::SinglyLinkedList<domain::DomainEventVisitor*> m_subscribers;
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_EVENT_BUS_HPP
