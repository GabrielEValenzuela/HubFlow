#include "hubflow/identity/application/usecases/queries/authenticate.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::application
{

    Authenticate::Authenticate(const UserRepository& users,
                               const TokenIssuer& tokens,
                               const domain::AccessPolicy& policy,
                               const shared::application::Clock& clock)
        : m_users {users}
        , m_tokens {tokens}
        , m_policy {policy}
        , m_clock {clock}
    {
    }

    shared::application::Principal Authenticate::execute(const std::string& token) const
    {
        const auto claims = m_tokens.verify(token, m_clock.now());
        const auto user = claims.has_value() ? m_users.byId(claims->userId) : std::nullopt;
        if (!user.has_value() || !user->active)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Unauthorized, "auth.invalid_token", "Sesión inválida o expirada"};
        }
        return shared::application::Principal {user->id, user->name, user->role, &m_policy};
    }

} // namespace hubflow::identity::application
