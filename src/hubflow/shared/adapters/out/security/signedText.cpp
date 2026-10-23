#include "hubflow/shared/adapters/out/security/signedText.hpp"

#include <array>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <stdexcept>

namespace hubflow::shared::adapters
{
namespace
{
constexpr const char* BASE64_URL_ALPHABET {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"};
} // namespace

std::string SignedText::base64UrlEncode(const std::string& input)
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

std::optional<std::string> SignedText::base64UrlDecode(const std::string& input)
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

std::string SignedText::sign(const std::string& secret, const std::string& text)
{
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest {};
    std::size_t                                length {0};
    const unsigned char*                       pResult = EVP_Q_mac(nullptr,
                                             "HMAC",
                                             nullptr,
                                             "SHA256",
                                             nullptr,
                                             secret.data(),
                                             secret.size(),
                                             reinterpret_cast<const unsigned char*>(text.data()),
                                             text.size(),
                                             digest.data(),
                                             digest.size(),
                                             &length);
    if (pResult == nullptr)
    {
        throw std::runtime_error("No se pudo firmar el token");
    }
    return base64UrlEncode(std::string {reinterpret_cast<const char*>(digest.data()), length});
}

bool SignedText::constantTimeEquals(const std::string& first, const std::string& second)
{
    return first.size() == second.size() && CRYPTO_memcmp(first.data(), second.data(), first.size()) == 0;
}

} // namespace hubflow::shared::adapters
