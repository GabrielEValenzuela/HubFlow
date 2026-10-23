#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_REGISTER_USER_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_REGISTER_USER_HPP

#include "hubflow/identity/application/ports/in/registerUserUseCase.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/identity/domain/services/passwordHasher.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/shared/domain/actorContext.hpp"

namespace hubflow::identity::application
{

/// @brief Implementación de RegisterUserUseCase.
class RegisterUser final : public RegisterUserUseCase
{
public:
    RegisterUser(UserRepository&                   users,
                 const domain::PasswordHasher&     hasher,
                 shared::application::EventBus&    bus,
                 const shared::application::Clock& clock);
    /// @brief Alta sin control de permisos, reservada para sembrar los usuarios iniciales del sistema.
    domain::UserState bootstrap(const RegisterUserRequest& request, const shared::domain::ActorContext& context);

private:
    domain::UserState execute(const shared::application::Principal& principal,
                              const RegisterUserRequest&            request) override;

    domain::UserState create(const RegisterUserRequest& request, const shared::domain::ActorContext& context);

    UserRepository&                   m_users;
    const domain::PasswordHasher&     m_hasher;
    shared::application::EventBus&    m_bus;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_REGISTER_USER_HPP
