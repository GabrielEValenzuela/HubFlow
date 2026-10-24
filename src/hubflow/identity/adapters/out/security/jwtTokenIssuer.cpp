#include "hubflow/identity/adapters/out/security/jwtTokenIssuer.hpp"

#include "hubflow/shared/adapters/out/security/signedText.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

#include <nlohmann/json.hpp>
#include <utility>

namespace hubflow::identity::adapters
{
    namespace
    {
        constexpr const char* JWT_HEADER {R"({"alg":"HS256","typ":"JWT"})"};
        using shared::adapters::SignedText;
    } // namespace

    JwtTokenIssuer::JwtTokenIssuer(std::string secret)
        : m_secret {std::move(secret)}
    {
    }

    std::string JwtTokenIssuer::issue(const application::TokenClaims& claims) const
    {
        const nlohmann::json payload {{"sub", std::to_string(claims.userId)},
                                      {"name", claims.name},
                                      {"role", shared::application::Vocabulary::name(claims.role)},
                                      {"exp", claims.expiresAt}};
        const std::string signingInput {SignedText::base64UrlEncode(JWT_HEADER) + "." +
                                        SignedText::base64UrlEncode(payload.dump())};
        return signingInput + "." + sign(signingInput);
    }

    std::optional<application::TokenClaims> JwtTokenIssuer::verify(const std::string& token,
                                                                   const std::int64_t now) const
    {
        std::optional<application::TokenClaims> claims;
        const auto firstDot = token.find('.');
        const auto secondDot = token.rfind('.');
        if (firstDot != std::string::npos && secondDot != firstDot)
        {
            const std::string signingInput {token.substr(0, secondDot)};
            const auto payload = SignedText::base64UrlDecode(token.substr(firstDot + 1, secondDot - firstDot - 1));
            if (SignedText::constantTimeEquals(sign(signingInput), token.substr(secondDot + 1)) && payload.has_value())
            {
                const auto json = nlohmann::json::parse(*payload, nullptr, false);
                const auto role =
                    json.is_object() ? shared::application::Vocabulary::role(json.value("role", "")) : std::nullopt;
                if (role.has_value() && json.value("exp", std::int64_t {0}) > now)
                {
                    claims = application::TokenClaims {std::stoull(json.value("sub", "0")),
                                                       json.value("name", ""),
                                                       *role,
                                                       json.value("exp", std::int64_t {0})};
                }
            }
        }
        return claims;
    }

    std::string JwtTokenIssuer::sign(const std::string& signingInput) const
    {
        return SignedText::sign(m_secret, signingInput);
    }

} // namespace hubflow::identity::adapters
