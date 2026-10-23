#include "hubflow/platform/http/healthController.hpp"

#include <utility>

namespace hubflow::platform
{

HealthController::HealthController(DatabaseProbe probe)
    : m_probe {std::move(probe)}
{
}

void HealthController::registerRoutes(ApiRouter& router)
{
    router.get(
        "/health",
        [this](const RequestContext&)
        {
            bool databaseUp {false};
            try
            {
                databaseUp = m_probe();
            }
            catch (const std::exception&)
            {
                databaseUp = false;
            }
            HttpResponse response;
            response.status = databaseUp ? HttpStatus::Ok : HttpStatus::ServiceUnavailable;
            response.body = Json {{"status", databaseUp ? "ok" : "degraded"}, {"database", databaseUp ? "ok" : "down"}};
            return response;
        },
        Access::Public);
}

} // namespace hubflow::platform
