#include "hubflow/routing/adapters/in/http/routeController.hpp"

#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/routing/adapters/in/http/routingJsonPresenter.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::routing::adapters
{
namespace
{
using platform::InputValidator;
using platform::Json;
using platform::RequestContext;
using shared::application::Vocabulary;

constexpr const char* ALGORITHMS {"BFS, DFS, GREEDY, ASTAR"};
constexpr const char* OUTCOMES {"ENTREGADO, REPROGRAMADO"};
constexpr std::size_t MAX_NOTE {200};

std::optional<domain::StopOutcome> resolvedOutcome(const std::string& text)
{
    const auto outcome = Vocabulary::stopOutcome(text);
    return outcome == domain::StopOutcome::Pending ? std::nullopt : outcome;
}

std::uint64_t routeId(const RequestContext& context)
{
    auto                input = InputValidator::parameters(context);
    const std::uint64_t id {input.requiredIdentifier("id")};
    input.validate();
    return id;
}
} // namespace

RouteController::RouteController(RouteUseCases useCases)
    : m_useCases {useCases}
{
}

void RouteController::registerRoutes(platform::ApiRouter& router)
{
    router.get("/routes",
               [this](const RequestContext& context)
               {
                   auto       input  = InputValidator::parameters(context);
                   const auto paging = input.page();
                   input.validate();
                   const auto routes = m_useCases.list.execute(context.user());
                   Json       items  = Json::array();
                   for (std::size_t i = 0; i < routes.size(); ++i)
                   {
                       items.push_back(RoutingJsonPresenter::routeSummary(*routes[i]));
                   }
                   return platform::HttpResponse::ok(page(items, paging.first, paging.second));
               });
    router.post(
        "/routes",
        [this](const RequestContext& context)
        {
            return assign(context);
        },
        application::AssignDeliveryRouteUseCase::IDEMPOTENCY);
    router.get("/routes/:id",
               [this](const RequestContext& context)
               {
                   return platform::HttpResponse::ok(RoutingJsonPresenter::courierSheet(
                       m_useCases.sheet.execute(context.user(), routeId(context)), m_useCases.roadMap.execute()));
               });
    router.patch("/routes/:id/stops/:order",
                 [this](const RequestContext& context)
                 {
                     return resolveStop(context);
                 });
    router.get("/routes/:id/initial-state",
               [this](const RequestContext& context)
               {
                   return download(context, true);
               });
    router.get("/routes/:id/sequence",
               [this](const RequestContext& context)
               {
                   return download(context, false);
               });
}

platform::HttpResponse RouteController::assign(const RequestContext& context) const
{
    auto                            input = InputValidator::body(context.request);
    application::AssignRouteRequest request;
    request.batchId   = input.requiredIdentifier("batchId");
    request.courierId = input.requiredIdentifier("courierId");
    request.algorithm =
        input.optionalTerm<domain::SearchAlgorithm>("algorithm", Vocabulary::searchAlgorithm, ALGORITHMS)
            .value_or(domain::SearchAlgorithm::AStar);
    input.validate();
    const auto& route = m_useCases.assign.execute(context.user(), request);
    return platform::HttpResponse::created(RoutingJsonPresenter::route(route, m_useCases.roadMap.execute()),
                                           locationOf("/routes/" + std::to_string(route.id())));
}

platform::HttpResponse RouteController::resolveStop(const RequestContext& context) const
{
    auto                path = InputValidator::parameters(context);
    const std::uint64_t id {path.requiredIdentifier("id")};
    const std::uint64_t order {path.requiredIdentifier("order")};
    path.validate();
    auto                            input = InputValidator::body(context.request);
    application::ResolveStopRequest request;
    request.routeId = id;
    request.order   = static_cast<std::size_t>(order);
    request.outcome = input.requiredTerm<domain::StopOutcome>("outcome", resolvedOutcome, OUTCOMES);
    request.note    = input.optionalText("note", MAX_NOTE).value_or("");
    input.validate();
    m_useCases.resolveStop.execute(context.user(), request);
    return platform::HttpResponse::ok(
        RoutingJsonPresenter::courierSheet(m_useCases.sheet.execute(context.user(), id), m_useCases.roadMap.execute()));
}

platform::HttpResponse RouteController::download(const RequestContext& context, const bool initialState) const
{
    const auto& route = m_useCases.get.execute(context.user(), routeId(context));
    const auto& map   = m_useCases.roadMap.execute();
    return initialState
               ? platform::HttpResponse::attachment(RoutingJsonPresenter::initialState(route, map),
                                                    "initial_state.json")
               : platform::HttpResponse::attachment(RoutingJsonPresenter::sequence(route, map), "sequence.json");
}

} // namespace hubflow::routing::adapters
