#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_SIGN_IN_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_SIGN_IN_HPP

#include "hubflow/identity/application/ports/in/signInUseCase.hpp"
#include "hubflow/identity/application/ports/out/tokenIssuer.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/identity/domain/services/passwordHasher.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

#include <cstdint>

namespace hubflow::identity::application
{

    /// @brief Implementación de SignInUseCase.
    class SignIn final : public SignInUseCase
    {
    public:
        SignIn(const UserRepository& users,
               const domain::PasswordHasher& hasher,
               const TokenIssuer& tokens,
               const shared::application::Clock& clock);
        static constexpr std::int64_t SESSION_SECONDS {8 * 60 * 60};

    private:
        Session execute(const std::string& email, const std::string& password) const override;

        const UserRepository& m_users;
        const domain::PasswordHasher& m_hasher;
        const TokenIssuer& m_tokens;
        const shared::application::Clock& m_clock;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_SIGN_IN_HPP
