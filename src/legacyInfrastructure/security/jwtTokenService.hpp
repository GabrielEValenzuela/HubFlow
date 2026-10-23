#ifndef HUBFLOW_INFRASTRUCTURE_SECURITY_JWT_TOKEN_SERVICE_HPP
#define HUBFLOW_INFRASTRUCTURE_SECURITY_JWT_TOKEN_SERVICE_HPP

#include "application/identity/tokenService.hpp"

namespace hubflow::infrastructure
{

/// @brief JSON Web Tokens firmados con HMAC-SHA256 (OpenSSL).
class JwtTokenService final : public application::TokenService
{
public:
    explicit JwtTokenService(std::string secret);

private:
    std::string                             issue(const application::TokenClaims& claims) const override;
    std::optional<application::TokenClaims> verify(const std::string& token, std::int64_t now) const override;

    std::string sign(const std::string& signingInput) const;

    std::string m_secret;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_SECURITY_JWT_TOKEN_SERVICE_HPP
