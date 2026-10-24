#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_TOKEN_ISSUER_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_TOKEN_ISSUER_HPP

#include "hubflow/identity/domain/valueobjects/role.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace hubflow::identity::application
{

    /// @brief Afirmaciones firmadas dentro de un token de sesión.
    struct TokenClaims
    {
        std::uint64_t userId {0};
        std::string name;
        domain::Role role {domain::Role::Operador};
        std::int64_t expiresAt {0};
    };

    /// @brief Emisión y verificación de tokens de sesión; la firma concreta la resuelve un adaptador.
    class TokenIssuer
    {
    public:
        virtual ~TokenIssuer() = default;

        virtual std::string issue(const TokenClaims& claims) const = 0;

        /// @brief Claims si la firma es válida y no expiró a @p now; vacío en otro caso.
        virtual std::optional<TokenClaims> verify(const std::string& token, std::int64_t now) const = 0;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_TOKEN_ISSUER_HPP
