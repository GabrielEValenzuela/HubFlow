#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_TRACKING_TOKEN_ISSUER_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_TRACKING_TOKEN_ISSUER_HPP

#include <cstdint>
#include <optional>
#include <string>

namespace hubflow::tracking::application
{

/// @brief Permiso para ver el detalle de un único paquete hasta un instante.
struct TrackingGrant
{
    std::string  code;
    std::int64_t expiresAt {0};
};

/**
 * @brief Emisión y verificación de las constancias de verificación del cliente; la firma la resuelve un adaptador.
 *
 * La constancia está atada a un código de seguimiento y vence sola: no sirve para ningún otro paquete ni después de
 * su vencimiento. Es independiente de las sesiones del personal.
 */
class TrackingTokenIssuer
{
public:
    virtual ~TrackingTokenIssuer() = default;

    virtual std::string issue(const TrackingGrant& grant) const = 0;

    /// @brief El permiso si la firma es válida y no venció a @p now; nada en cualquier otro caso.
    virtual std::optional<TrackingGrant> verify(const std::string& token, std::int64_t now) const = 0;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_TRACKING_TOKEN_ISSUER_HPP
