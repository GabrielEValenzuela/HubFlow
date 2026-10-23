#ifndef HUBFLOW_SHARED_APPLICATION_PORTS_OUT_RATE_LIMITER_HPP
#define HUBFLOW_SHARED_APPLICATION_PORTS_OUT_RATE_LIMITER_HPP

#include <cstdint>
#include <string>

namespace hubflow::shared::application
{

/// @brief Respuesta del limitador: si el intento puede seguir y, si no, cuántos segundos faltan para el próximo.
struct RateDecision
{
    bool          allowed {true};
    std::uint32_t retryAfterSeconds {0};
};

/**
 * @brief Límite de intentos por clave dentro de una ventana de tiempo (puerto).
 *
 * La clave la elige quien lo usa; por ejemplo, la dirección de quien consulta junto con el código que consulta, de
 * modo que equivocarse con un paquete no bloquee a esa persona para los demás.
 */
class RateLimiter
{
public:
    virtual ~RateLimiter() = default;

    /**
     * @brief Registra un intento de @p key a las @p now (segundos desde epoch) y dice si puede continuar.
     *
     * Un intento rechazado no se suma: el bloqueo termina cuando el intento permitido más antiguo sale de la ventana.
     */
    virtual RateDecision attempt(const std::string& key, std::int64_t now) = 0;
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_PORTS_OUT_RATE_LIMITER_HPP
