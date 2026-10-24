#include "hubflow/logistics/adapters/in/http/operationsController.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::adapters
{
    namespace
    {
        using platform::InputValidator;
        using platform::RequestContext;

        std::string zoneOf(const RequestContext& context)
        {
            auto input = InputValidator::parameters(context);
            const auto zone = input.code("zone");
            input.validate();
            return zone;
        }
    } // namespace

    OperationsController::OperationsController(const OperationsUseCases& useCases)
        : m_useCases {useCases}
    {
    }

    void OperationsController::registerRoutes(platform::ApiRouter& router)
    {
        router.get("/zones/:zone/summary",
                   [this](const RequestContext& context)
                   {
                       const auto zone = zoneOf(context);
                       return platform::HttpResponse::ok(LogisticsJsonPresenter::zoneSummary(
                           zone, m_useCases.summarizeZone.execute(context.user(), zone)));
                   });
        router.get("/zones/:zone/heaviest-shipment",
                   [this](const RequestContext& context)
                   {
                       const auto zone = zoneOf(context);
                       const auto* pShipment = m_useCases.findHeaviest.execute(context.user(), zone);
                       if (pShipment == nullptr)
                       {
                           throw shared::domain::BusinessError {shared::domain::ErrorKind::NotFound,
                                                                "zone.empty",
                                                                "No hay envíos pendientes para la zona seleccionada."};
                       }
                       return platform::HttpResponse::ok(LogisticsJsonPresenter::shipmentSummary(*pShipment));
                   });
        router.get("/metrics",
                   [this](const RequestContext& context)
                   {
                       return platform::HttpResponse::ok(
                           LogisticsJsonPresenter::metrics(m_useCases.metrics.execute(context.user())));
                   });
    }

} // namespace hubflow::logistics::adapters
