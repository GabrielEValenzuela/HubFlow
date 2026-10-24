#ifndef HUBFLOW_SHARED_ADAPTERS_OUT_SECURITY_SIGNED_TEXT_HPP
#define HUBFLOW_SHARED_ADAPTERS_OUT_SECURITY_SIGNED_TEXT_HPP

#include <optional>
#include <string>

namespace hubflow::shared::adapters
{

    /// @brief Herramientas de firma compartidas por los emisores de tokens: Base64 URL y HMAC-SHA256 (OpenSSL).
    class SignedText final
    {
    public:
        /// @brief Codifica en Base64 URL sin relleno.
        static std::string base64UrlEncode(const std::string& input);

        /// @brief Decodifica Base64 URL; nada si trae caracteres ajenos al alfabeto.
        static std::optional<std::string> base64UrlDecode(const std::string& input);

        /// @brief Firma HMAC-SHA256 de @p text con @p secret, en Base64 URL.
        /// @throws std::runtime_error si OpenSSL no puede firmar.
        static std::string sign(const std::string& secret, const std::string& text);

        /// @brief Igualdad en tiempo constante.
        static bool constantTimeEquals(const std::string& first, const std::string& second);
    };

} // namespace hubflow::shared::adapters

#endif // HUBFLOW_SHARED_ADAPTERS_OUT_SECURITY_SIGNED_TEXT_HPP
