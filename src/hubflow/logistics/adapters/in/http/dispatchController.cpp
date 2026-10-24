#include "hubflow/logistics/adapters/in/http/dispatchController.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"

namespace hubflow::logistics::adapters
{
    namespace
    {
        constexpr std::size_t MAX_NOTE {200};
    } // namespace

    DispatchController::DispatchController(application::DispatchNextShipmentUseCase& dispatchNext,
                                           const application::ListPendingShipmentsUseCase& listPending)
        : m_dispatchNext {dispatchNext}
        , m_listPending {listPending}
    {
    }

    void DispatchController::registerRoutes(platform::ApiRouter& router)
    {
        router.get("/pending-shipments",
                   [this](const platform::RequestContext& context)
                   {
                       auto input = platform::InputValidator::parameters(context);
                       const auto paging = input.page();
                       input.validate();
                       platform::Json items = platform::Json::array();
                       m_listPending.execute(context.user())
                           .forEach([&items](const auto& shipment)
                                    { items.push_back(LogisticsJsonPresenter::shipmentSummary(shipment)); });
                       return platform::HttpResponse::ok(page(items, paging.first, paging.second));
                   });
        router.post(
            "/dispatches",
            [this](const platform::RequestContext& context)
            {
                auto input = platform::InputValidator::body(context.request);
                const std::string note {input.optionalText("note", MAX_NOTE).value_or("")};
                input.validate();
                const auto& shipment = m_dispatchNext.execute(context.user(), note);
                return platform::HttpResponse::created(LogisticsJsonPresenter::shipment(shipment),
                                                       locationOf("/shipments/" + shipment.code()));
            },
            application::DispatchNextShipmentUseCase::IDEMPOTENCY);
    }

} // namespace hubflow::logistics::adapters
