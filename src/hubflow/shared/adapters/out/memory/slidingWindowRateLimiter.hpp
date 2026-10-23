#ifndef HUBFLOW_SHARED_ADAPTERS_OUT_MEMORY_SLIDING_WINDOW_RATE_LIMITER_HPP
#define HUBFLOW_SHARED_ADAPTERS_OUT_MEMORY_SLIDING_WINDOW_RATE_LIMITER_HPP

#include "hubflow/shared/application/ports/out/rateLimiter.hpp"

#include <deque>
#include <map>

namespace hubflow::shared::adapters
{

/**
 * @brief Limitador en memoria con ventana deslizante: permite hasta @c maxAttempts intentos por clave en los últimos
 * @c windowSeconds segundos.
 *
 * Los contadores viven sólo en el proceso (se pierden al reiniciar). Las claves sin intentos recientes se descartan
 * para que la memoria no crezca con cada dirección y código consultados. No es seguro entre hilos por sí mismo: el
 * router HTTP ya serializa los pedidos.
 */
class SlidingWindowRateLimiter final : public application::RateLimiter
{
public:
    SlidingWindowRateLimiter(std::size_t maxAttempts, std::int64_t windowSeconds);

private:
    application::RateDecision attempt(const std::string& key, std::int64_t now) override;

    void discardExpired(std::int64_t now);

    std::size_t                                     m_maxAttempts;
    std::int64_t                                    m_windowSeconds;
    std::map<std::string, std::deque<std::int64_t>> m_attempts;
};

} // namespace hubflow::shared::adapters

#endif // HUBFLOW_SHARED_ADAPTERS_OUT_MEMORY_SLIDING_WINDOW_RATE_LIMITER_HPP
