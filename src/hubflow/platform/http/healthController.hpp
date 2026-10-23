#ifndef HUBFLOW_PLATFORM_HTTP_HEALTH_CONTROLLER_HPP
#define HUBFLOW_PLATFORM_HTTP_HEALTH_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"

#include <functional>

namespace hubflow::platform
{

/// @brief Pregunta si la base de datos responde.
using DatabaseProbe = std::function<bool()>;

/**
 * @brief Estado del servicio para orquestadores y para esperar el arranque.
 *
 * - @c GET /health (público): responde 200 con {status, database} si la base de datos responde y 503 si no.
 */
class HealthController final : public Controller
{
public:
    explicit HealthController(DatabaseProbe probe);

private:
    void registerRoutes(ApiRouter& router) override;

    DatabaseProbe m_probe;
};

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_HTTP_HEALTH_CONTROLLER_HPP
