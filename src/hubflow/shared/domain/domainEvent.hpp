#ifndef HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_HPP
#define HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_HPP

#include "hubflow/shared/domain/actorContext.hpp"

namespace hubflow::shared::domain
{

    class DomainEventVisitor;

    /**
     * @brief Hecho de negocio ya ocurrido. Inmutable.
     *
     * Usa double dispatch (@c accept) para que cada suscriptor reaccione sólo a los eventos que
     * le interesan sin @c dynamic_cast ni @c switch por tipo.
     */
    class DomainEvent
    {
    public:
        explicit DomainEvent(ActorContext context);

        virtual ~DomainEvent() = default;

        /// @brief Despacha el evento al método @c on(...) correspondiente del visitante.
        virtual void accept(DomainEventVisitor& visitor) const = 0;

        const ActorContext& actorContext() const noexcept;

    private:
        ActorContext m_context;
    };

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_HPP
