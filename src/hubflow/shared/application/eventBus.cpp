#include "hubflow/shared/application/eventBus.hpp"

namespace hubflow::shared::application
{

void EventBus::subscribe(domain::DomainEventVisitor& subscriber)
{
    m_subscribers.pushBack(&subscriber);
}

void EventBus::publish(const domain::DomainEvent& event)
{
    m_subscribers.forEach(
        [&event](domain::DomainEventVisitor* pSubscriber)
        {
            event.accept(*pSubscriber);
        });
}

} // namespace hubflow::shared::application
