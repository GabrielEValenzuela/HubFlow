#include "hubflow/identity/application/usecases/commands/signIn.hpp"

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/identity/domain/valueobjects/email.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::application
{
    namespace
    {
        shared::domain::BusinessError invalidCredentials()
        {
            return shared::domain::BusinessError {
                shared::domain::ErrorKind::Unauthorized, "auth.invalid_credentials", "Email o contraseña incorrectos"};
        }
    } // namespace

    SignIn::SignIn(const UserRepository& users,
                   const domain::PasswordHasher& hasher,
                   const TokenIssuer& tokens,
                   const shared::application::Clock& clock)
        : m_users {users}
        , m_hasher {hasher}
        , m_tokens {tokens}
        , m_clock {clock}
    {
    }

    Session SignIn::execute(const std::string& email, const std::string& password) const
    {
        const auto state = m_users.byEmail(domain::Email {email}.value());
        if (!state.has_value())
        {
            throw invalidCredentials();
        }
        const domain::User* pUser = domain::User::restore(*state);
        const bool authenticated {pUser->authenticates(password, m_hasher)};
        delete pUser;
        if (!authenticated)
        {
            throw invalidCredentials();
        }
        const std::int64_t expiresAt {m_clock.now() + SESSION_SECONDS};
        return Session {
            m_tokens.issue(TokenClaims {state->id, state->name, state->role, expiresAt}), expiresAt, *state};
    }

} // namespace hubflow::identity::application
