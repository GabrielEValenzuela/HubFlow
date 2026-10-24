#ifndef HUBFLOW_IDENTITY_DOMAIN_EVENTS_USER_EVENTS_HPP
#define HUBFLOW_IDENTITY_DOMAIN_EVENTS_USER_EVENTS_HPP

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::identity::domain
{

    /// @brief Base de los eventos de usuario: estado resultante.
    class UserEvent : public shared::domain::DomainEvent
    {
    public:
        UserEvent(UserState state, shared::domain::ActorContext context);

        const UserState& state() const noexcept;

    private:
        UserState m_state;
    };

    /// @brief Se dio de alta un usuario.
    class UserRegistered final : public UserEvent
    {
    public:
        using UserEvent::UserEvent;

    private:
        void accept(shared::domain::DomainEventVisitor& visitor) const override;
    };

    /// @brief Cambió el nombre, rol, contraseña o estado de un usuario.
    class UserProfileChanged final : public UserEvent
    {
    public:
        using UserEvent::UserEvent;

    private:
        void accept(shared::domain::DomainEventVisitor& visitor) const override;
    };

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_EVENTS_USER_EVENTS_HPP
