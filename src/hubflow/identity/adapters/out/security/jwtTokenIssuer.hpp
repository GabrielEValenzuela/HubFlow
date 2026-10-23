#ifndef HUBFLOW_IDENTITY_ADAPTERS_OUT_SECURITY_JWT_TOKEN_ISSUER_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_OUT_SECURITY_JWT_TOKEN_ISSUER_HPP

#include "hubflow/identity/application/ports/out/tokenIssuer.hpp"

namespace hubflow::identity::adapters
{

/// @brief Tokens de sesión JSON Web Token firmados con HMAC-SHA256 (OpenSSL).
class JwtTokenIssuer final : public application::TokenIssuer
{
public:
    explicit JwtTokenIssuer(std::string secret);

private:
    std::string                             issue(const application::TokenClaims& claims) const override;
    std::optional<application::TokenClaims> verify(const std::string& token, std::int64_t now) const override;

    std::string sign(const std::string& signingInput) const;

    std::string m_secret;
};

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_OUT_SECURITY_JWT_TOKEN_ISSUER_HPP
