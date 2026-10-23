#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_AUTHENTICATE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_AUTHENTICATE_HPP

#include "hubflow/identity/application/ports/in/authenticateUseCase.hpp"
#include "hubflow/identity/application/ports/out/tokenIssuer.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::identity::application
{

/// @brief Implementación de AuthenticateUseCase.
class Authenticate final : public AuthenticateUseCase
{
public:
    Authenticate(const UserRepository&             users,
                 const TokenIssuer&                tokens,
                 const domain::AccessPolicy&       policy,
                 const shared::application::Clock& clock);

private:
    shared::application::Principal execute(const std::string& token) const override;

    const UserRepository&             m_users;
    const TokenIssuer&                m_tokens;
    const domain::AccessPolicy&       m_policy;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_AUTHENTICATE_HPP
