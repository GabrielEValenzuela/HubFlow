#ifndef HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_PUBLISHER_HPP
#define HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_PUBLISHER_HPP

#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::shared::domain
{

/// @brief Puerto por el que un agregado entrega sus eventos al resto del sistema.
class DomainEventPublisher
{
public:
    virtual ~DomainEventPublisher() = default;

    /// @brief Publica un evento; el publicador no toma ownership.
    virtual void publish(const DomainEvent& event) = 0;
};

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_DOMAIN_EVENT_PUBLISHER_HPP
