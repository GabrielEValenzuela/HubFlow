#include "hubflow/routing/adapters/in/http/roadMapController.hpp"

#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/routing/adapters/in/http/routingJsonPresenter.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::routing::adapters
{
namespace
{
using shared::application::Vocabulary;

constexpr const char* ALGORITHMS {"BFS, DFS, GREEDY, ASTAR"};
constexpr std::size_t MAX_NODE_ID {16};
} // namespace

RoadMapController::RoadMapController(const application::GetRoadMapUseCase&   getRoadMap,
                                     const application::PreviewRouteUseCase& preview)
    : m_getRoadMap {getRoadMap}
    , m_preview {preview}
{
}

void RoadMapController::registerRoutes(platform::ApiRouter& router)
{
    router.get("/road-map",
               [this](const platform::RequestContext&)
               {
                   return platform::HttpResponse::ok(RoutingJsonPresenter::roadMap(m_getRoadMap.execute()));
               });
    router.get("/route-previews",
               [this](const platform::RequestContext& context)
               {
                   auto              input = platform::InputValidator::parameters(context);
                   const std::string from {input.requiredText("from", 1, MAX_NODE_ID)};
                   const std::string to {input.requiredText("to", 1, MAX_NODE_ID)};
                   const auto        algorithm =
                       input.optionalTerm<domain::SearchAlgorithm>("algorithm", Vocabulary::searchAlgorithm, ALGORITHMS)
                           .value_or(domain::SearchAlgorithm::AStar);
                   input.validate();
                   const auto trace = m_preview.execute(context.user(), from, to, algorithm);
                   return platform::HttpResponse::ok(RoutingJsonPresenter::searchTrace(trace, m_getRoadMap.execute()));
               });
}

} // namespace hubflow::routing::adapters
