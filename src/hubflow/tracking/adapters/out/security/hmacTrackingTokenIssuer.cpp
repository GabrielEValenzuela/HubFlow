#include "hubflow/tracking/adapters/out/security/hmacTrackingTokenIssuer.hpp"

#include "hubflow/shared/adapters/out/security/signedText.hpp"

#include <nlohmann/json.hpp>

namespace hubflow::tracking::adapters
{
    namespace
    {
        using shared::adapters::SignedText;

        constexpr const char* KEY_LABEL {"|hubflow-customer-tracking-v1"};
    } // namespace

    HmacTrackingTokenIssuer::HmacTrackingTokenIssuer(const std::string& secret)
        : m_key {secret + KEY_LABEL}
    {
    }

    std::string HmacTrackingTokenIssuer::issue(const application::TrackingGrant& grant) const
    {
        const nlohmann::json payload {{"code", grant.code}, {"exp", grant.expiresAt}};
        const std::string body {SignedText::base64UrlEncode(payload.dump())};
        return body + "." + SignedText::sign(m_key, body);
    }

    std::optional<application::TrackingGrant> HmacTrackingTokenIssuer::verify(const std::string& token,
                                                                              const std::int64_t now) const
    {
        std::optional<application::TrackingGrant> grant;
        const auto dot = token.find('.');
        if (dot != std::string::npos &&
            SignedText::constantTimeEquals(SignedText::sign(m_key, token.substr(0, dot)), token.substr(dot + 1)))
        {
            const auto payload = SignedText::base64UrlDecode(token.substr(0, dot));
            const auto json = payload.has_value() ? nlohmann::json::parse(*payload, nullptr, false) : nlohmann::json {};
            if (json.is_object() && json.value("exp", std::int64_t {0}) > now)
            {
                grant = application::TrackingGrant {json.value("code", ""), json.value("exp", std::int64_t {0})};
            }
        }
        return grant;
    }

} // namespace hubflow::tracking::adapters
