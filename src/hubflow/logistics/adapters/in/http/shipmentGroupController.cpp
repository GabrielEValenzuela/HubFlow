#include "hubflow/logistics/adapters/in/http/shipmentGroupController.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/logistics/domain/valueobjects/uuidText.hpp"
#include "hubflow/platform/http/inputValidator.hpp"

namespace hubflow::logistics::adapters
{
    namespace
    {
        using platform::InputValidator;
        using platform::Json;
        using platform::RequestContext;

        constexpr std::size_t MIN_MEMBERS {2};
        constexpr std::size_t MAX_MEMBERS {20};

        std::string groupId(const RequestContext& context)
        {
            auto input = InputValidator::parameters(context);
            const std::string id {input.requiredText("id", domain::UuidText::LENGTH, domain::UuidText::LENGTH)};
            input.validate();
            return id;
        }
    } // namespace

    ShipmentGroupController::ShipmentGroupController(const ShipmentGroupUseCases& useCases)
        : m_useCases {useCases}
    {
    }

    void ShipmentGroupController::registerRoutes(platform::ApiRouter& router)
    {
        router.get("/shipment-groups",
                   [this](const RequestContext& context)
                   {
                       auto input = InputValidator::parameters(context);
                       const auto paging = input.page();
                       input.validate();
                       Json items = Json::array();
                       m_useCases.list.execute(context.user())
                           .forEach([&items](const auto& group)
                                    { items.push_back(LogisticsJsonPresenter::group(group)); });
                       return platform::HttpResponse::ok(page(items, paging.first, paging.second));
                   });
        router.post(
            "/shipment-groups",
            [this](const RequestContext& context)
            {
                auto input = InputValidator::body(context.request);
                ds::DynamicArray<std::string> codes;
                for (const auto& code : input.trackingCodes("codes", MIN_MEMBERS, MAX_MEMBERS))
                {
                    codes.pushBack(code);
                }
                input.validate();
                const auto& group = m_useCases.form.execute(context.user(), codes);
                return platform::HttpResponse::created(LogisticsJsonPresenter::group(group),
                                                       locationOf("/shipment-groups/" + group.id()));
            },
            application::FormShipmentGroupUseCase::IDEMPOTENCY);
        router.get("/shipment-groups/:id",
                   [this](const RequestContext& context)
                   {
                       return platform::HttpResponse::ok(
                           LogisticsJsonPresenter::group(m_useCases.get.execute(context.user(), groupId(context))));
                   });
        router.remove("/shipment-groups/:id",
                      [this](const RequestContext& context)
                      {
                          const std::string id {groupId(context)};
                          m_useCases.dissolve.execute(context.user(), id);
                          return platform::HttpResponse::ok(Json {{"id", id}, {"dissolved", true}});
                      });
    }

} // namespace hubflow::logistics::adapters
