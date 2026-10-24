#include "hubflow/logistics/adapters/in/http/catalogController.hpp"

#include "hubflow/logistics/adapters/in/http/catalogJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"

namespace hubflow::logistics::adapters
{
    namespace
    {
        using platform::HttpResponse;
        using platform::InputValidator;
        using platform::RequestContext;

        constexpr std::size_t MIN_NAME {2};
        constexpr std::size_t MAX_NAME {60};

        template<typename TRequest>
        TRequest changesOf(const RequestContext& context)
        {
            auto input = InputValidator::body(context.request);
            TRequest request;
            if (input.has("name"))
            {
                request.name = input.requiredText("name", MIN_NAME, MAX_NAME);
            }
            request.active = input.optionalFlag("active");
            input.validate();
            return request;
        }

        std::string codeOf(const RequestContext& context)
        {
            auto input = InputValidator::parameters(context);
            const std::string code {input.code("code")};
            input.validate();
            return code;
        }
    } // namespace

    CatalogController::CatalogController(const CatalogUseCases& useCases)
        : m_useCases {useCases}
    {
    }

    void CatalogController::registerRoutes(platform::ApiRouter& router)
    {
        router.get(
            "/zones",
            [this](const RequestContext& context)
            { return HttpResponse::ok(CatalogJsonPresenter::zones(m_useCases.listZones.execute(context.user()))); });
        router.post(
            "/zones",
            [this](const RequestContext& context)
            {
                auto input = InputValidator::body(context.request);
                application::CreateZoneRequest request;
                request.code = input.code("code");
                request.name = input.requiredText("name", MIN_NAME, MAX_NAME);
                input.validate();
                const auto& zone = m_useCases.createZone.execute(context.user(), request);
                return HttpResponse::created(CatalogJsonPresenter::zone(zone), locationOf("/zones/" + zone.code()));
            },
            application::CreateZoneUseCase::IDEMPOTENCY);
        router.patch("/zones/:code",
                     [this](const RequestContext& context)
                     {
                         return HttpResponse::ok(CatalogJsonPresenter::zone(m_useCases.updateZone.execute(
                             context.user(), codeOf(context), changesOf<application::UpdateZoneRequest>(context))));
                     });

        router.get("/service-levels",
                   [this](const RequestContext& context)
                   {
                       return HttpResponse::ok(
                           CatalogJsonPresenter::serviceLevels(m_useCases.listServices.execute(context.user())));
                   });
        router.post(
            "/service-levels",
            [this](const RequestContext& context)
            {
                auto input = InputValidator::body(context.request);
                application::CreateServiceLevelRequest request;
                request.code = input.code("code");
                request.name = input.requiredText("name", MIN_NAME, MAX_NAME);
                request.priority = static_cast<unsigned>(input.requiredIdentifier("priority"));
                input.validate();
                const auto& service = m_useCases.createService.execute(context.user(), request);
                return HttpResponse::created(CatalogJsonPresenter::serviceLevel(service),
                                             locationOf("/service-levels/" + service.code()));
            },
            application::CreateServiceLevelUseCase::IDEMPOTENCY);
        router.patch(
            "/service-levels/:code",
            [this](const RequestContext& context)
            {
                return HttpResponse::ok(CatalogJsonPresenter::serviceLevel(m_useCases.updateService.execute(
                    context.user(), codeOf(context), changesOf<application::UpdateServiceLevelRequest>(context))));
            });

        router.get("/shipment-features",
                   [this](const RequestContext& context)
                   {
                       return HttpResponse::ok(
                           CatalogJsonPresenter::features(m_useCases.listFeatures.execute(context.user())));
                   });
        router.post(
            "/shipment-features",
            [this](const RequestContext& context)
            {
                auto input = InputValidator::body(context.request);
                application::CreateShipmentFeatureRequest request;
                request.code = input.code("code");
                request.name = input.requiredText("name", MIN_NAME, MAX_NAME);
                input.validate();
                const auto& feature = m_useCases.createFeature.execute(context.user(), request);
                return HttpResponse::created(CatalogJsonPresenter::feature(feature),
                                             locationOf("/shipment-features/" + feature.code()));
            },
            application::CreateShipmentFeatureUseCase::IDEMPOTENCY);
        router.patch(
            "/shipment-features/:code",
            [this](const RequestContext& context)
            {
                return HttpResponse::ok(CatalogJsonPresenter::feature(m_useCases.updateFeature.execute(
                    context.user(), codeOf(context), changesOf<application::UpdateShipmentFeatureRequest>(context))));
            });
    }

} // namespace hubflow::logistics::adapters
