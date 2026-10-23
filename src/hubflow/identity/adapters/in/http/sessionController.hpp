#ifndef HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_SESSION_CONTROLLER_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_SESSION_CONTROLLER_HPP

#include "hubflow/identity/application/ports/in/findUserUseCase.hpp"
#include "hubflow/identity/application/ports/in/signInUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::identity::adapters
{

/**
 * @brief Sesiones de usuario.
 *
 * - @c POST /sessions: inicia sesión con email y contraseña (público, no admite @c Idempotency-Key).
 * - @c GET /sessions/current: usuario de la sesión actual.
 */
class SessionController final : public platform::Controller
{
public:
    SessionController(const application::SignInUseCase& signIn, const application::FindUserUseCase& findUser);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    const application::SignInUseCase&   m_signIn;
    const application::FindUserUseCase& m_findUser;
};

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_SESSION_CONTROLLER_HPP
