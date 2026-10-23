#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_SECURITY_HMAC_TRACKING_TOKEN_ISSUER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_SECURITY_HMAC_TRACKING_TOKEN_ISSUER_HPP

#include "hubflow/tracking/application/ports/out/trackingTokenIssuer.hpp"

namespace hubflow::tracking::adapters
{

/**
 * @brief Constancias de verificación firmadas con HMAC-SHA256 (OpenSSL).
 *
 * La constancia lleva el código de seguimiento y el vencimiento. La clave de firma se deriva del secreto de la
 * aplicación con una etiqueta propia, de modo que una constancia de seguimiento nunca sirve como sesión del personal
 * ni al revés.
 */
class HmacTrackingTokenIssuer final : public application::TrackingTokenIssuer
{
public:
    explicit HmacTrackingTokenIssuer(const std::string& secret);

private:
    std::string                               issue(const application::TrackingGrant& grant) const override;
    std::optional<application::TrackingGrant> verify(const std::string& token, std::int64_t now) const override;

    std::string m_key;
};

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_SECURITY_HMAC_TRACKING_TOKEN_ISSUER_HPP
