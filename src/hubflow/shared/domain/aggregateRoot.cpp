#include "hubflow/shared/domain/aggregateRoot.hpp"

namespace hubflow::shared::domain
{

    AggregateRoot::~AggregateRoot()
    {
        m_events.forEach([](const DomainEvent* pEvent) { delete pEvent; });
    }

    void AggregateRoot::publishEvents(DomainEventPublisher& publisher)
    {
        while (!m_events.isEmpty())
        {
            const DomainEvent* pEvent = m_events.dequeue();
            try
            {
                publisher.publish(*pEvent);
            }
            catch (...)
            {
                delete pEvent;
                throw;
            }
            delete pEvent;
        }
    }

    std::size_t AggregateRoot::pendingEvents() const noexcept
    {
        return m_events.size();
    }

    void AggregateRoot::raise(const DomainEvent* pEvent)
    {
        m_events.enqueue(pEvent);
    }

} // namespace hubflow::shared::domain
