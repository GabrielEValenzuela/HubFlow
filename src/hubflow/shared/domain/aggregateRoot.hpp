#ifndef HUBFLOW_SHARED_DOMAIN_AGGREGATE_ROOT_HPP
#define HUBFLOW_SHARED_DOMAIN_AGGREGATE_ROOT_HPP

#include "datastructures/queue.hpp"
#include "hubflow/shared/domain/domainEventPublisher.hpp"

namespace hubflow::shared::domain
{

    /**
     * @brief Base de los agregados: acumula los eventos que genera su comportamiento.
     *
     * Ownership: el agregado es dueño de sus eventos pendientes (creados con @c new). Al
     * publicarlos los entrega por referencia y luego los destruye; si nunca se publican, el
     * destructor los libera.
     */
    class AggregateRoot
    {
    public:
        AggregateRoot() = default;

        virtual ~AggregateRoot();

        AggregateRoot(const AggregateRoot&) = delete;
        AggregateRoot& operator=(const AggregateRoot&) = delete;

        /// @brief Publica en orden FIFO los eventos pendientes y los libera.
        void publishEvents(DomainEventPublisher& publisher);

        std::size_t pendingEvents() const noexcept;

    protected:
        /// @brief Registra un evento; el agregado toma ownership de @p pEvent.
        void raise(const DomainEvent* pEvent);

    private:
        ds::Queue<const DomainEvent*> m_events;
    };

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_AGGREGATE_ROOT_HPP
