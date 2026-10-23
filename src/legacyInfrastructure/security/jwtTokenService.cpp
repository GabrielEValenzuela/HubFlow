#include "infrastructure/security/jwtTokenService.hpp"

#include "application/shared/vocabulary.hpp"

#include <nlohmann/json.hpp>
#include <openssl/crypto.h>
#include <openssl/evp.h>

#include <array>
#include <stdexcept>
#include <utility>

namespace hubflow::infrastructure
{
namespace
{
constexpr const char* JWT_HEADER {R"({"alg":"HS256","typ":"JWT"})"};
constexpr const char* BASE64_URL_ALPHABET {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"};

std::string base64UrlEncode(const std::string& input)
{
    std::string  output;
    unsigned int buffer {0};
    int          bits {0};
    for (const unsigned char byte : input)
    {
        buffer = (buffer << 8U) | byte;
        bits += 8;
        while (bits >= 6)
        {
            bits -= 6;
            output.push_back(BASE64_URL_ALPHABET[(buffer >> static_cast<unsigned>(bits)) & 0x3FU]);
        }
    }
    if (bits > 0)
    {
        output.push_back(BASE64_URL_ALPHABET[(buffer << static_cast<unsigned>(6 - bits)) & 0x3FU]);
    }
    return output;
}

std::optional<std::string> base64UrlDecode(const std::string& input)
{
    std::optional<std::string> output {std::string {}};
    unsigned int               buffer {0};
    int                        bits {0};
    const std::string          alphabet {BASE64_URL_ALPHABET};
    for (const char character : input)
    {
        const auto index = alphabet.find(character);
        if (index == std::string::npos)
        {
            output.reset();
            break;
        }
        buffer = (buffer << 6U) | static_cast<unsigned>(index);
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            output->push_back(static_cast<char>((buffer >> static_cast<unsigned>(bits)) & 0xFFU));
        }
    }
    return output;
}

bool constantTimeEquals(const std::string& a, const std::string& b)
{
    return a.size() == b.size() && CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
}
} // namespace

JwtTokenService::JwtTokenService(std::string secret)
    : m_secret {std::move(secret)}
{
}

std::string JwtTokenService::issue(const application::TokenClaims& claims) const
{
    const nlohmann::json payload {{"sub", std::to_string(claims.userId)},
                                  {"name", claims.name},
                                  {"role", application::Vocabulary::name(claims.role)},
                                  {"exp", claims.expiresAt}};
    const std::string signingInput {base64UrlEncode(JWT_HEADER) + "." + base64UrlEncode(payload.dump())};
    return signingInput + "." + sign(signingInput);
}

std::optional<application::TokenClaims> JwtTokenService::verify(const std::string& token, const std::int64_t now) const
{
    std::optional<application::TokenClaims> claims;
    const auto                              firstDot  = token.find('.');
    const auto                              secondDot = token.rfind('.');
    if (firstDot != std::string::npos && secondDot != firstDot)
    {
        const std::string signingInput {token.substr(0, secondDot)};
        const auto        payload = base64UrlDecode(token.substr(firstDot + 1, secondDot - firstDot - 1));
        if (constantTimeEquals(sign(signingInput), token.substr(secondDot + 1)) && payload.has_value())
        {
            const auto json = nlohmann::json::parse(*payload, nullptr, false);
            const auto role = json.is_object() ? application::Vocabulary::role(json.value("role", "")) : std::nullopt;
            if (role.has_value() && json.value("exp", std::int64_t {0}) > now)
            {
                claims = application::TokenClaims {
                    std::stoull(json.value("sub", "0")), json.value("name", ""), *role, json.value("exp", std::int64_t {0})};
            }
        }
    }
    return claims;
}

std::string JwtTokenService::sign(const std::string& signingInput) const
{
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest {};
    std::size_t                                length {0};
    const unsigned char* pResult = EVP_Q_mac(nullptr,
                                             "HMAC",
                                             nullptr,
                                             "SHA256",
                                             nullptr,
                                             m_secret.data(),
                                             m_secret.size(),
                                             reinterpret_cast<const unsigned char*>(signingInput.data()),
                                             signingInput.size(),
                                             digest.data(),
                                             digest.size(),
                                             &length);
    if (pResult == nullptr)
    {
        throw std::runtime_error("No se pudo firmar el token");
    }
    return base64UrlEncode(std::string {reinterpret_cast<const char*>(digest.data()), length});
}

} // namespace hubflow::infrastructure
