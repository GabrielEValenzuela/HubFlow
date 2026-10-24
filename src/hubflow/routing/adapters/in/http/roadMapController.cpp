#include "hubflow/routing/adapters/in/http/roadMapController.hpp"

#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/routing/adapters/in/http/routingJsonPresenter.hpp"
#include "hubflow/routing/domain/services/travelTimePolicy.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::routing::adapters
{
    namespace
    {
        using shared::application::Vocabulary;

        constexpr const char* ALGORITHMS {"BFS, DFS, GREEDY, ASTAR"};
        constexpr std::size_t MAX_NODE_ID {16};
        constexpr std::size_t MAX_LABEL {80};
        constexpr double MIN_COORDINATE {-1000.0};
        constexpr double MAX_COORDINATE {1000.0};
    } // namespace

    RoadMapController::RoadMapController(const application::GetRoadMapUseCase& getRoadMap,
                                         const application::PreviewRouteUseCase& preview,
                                         application::AddRoadNodeUseCase& addNode,
                                         application::UpdateRoadNodeUseCase& updateNode)
        : m_getRoadMap {getRoadMap}
        , m_preview {preview}
        , m_addNode {addNode}
        , m_updateNode {updateNode}
    {
    }

    void RoadMapController::registerRoutes(platform::ApiRouter& router)
    {
        router.get("/road-map",
                   [this](const platform::RequestContext&)
                   {
                       platform::Json body = RoutingJsonPresenter::roadMap(m_getRoadMap.execute());
                       body["travelEstimate"] =
                           platform::Json {{"avgKmh", domain::TravelTimePolicy::AVERAGE_KMH},
                                           {"minutesPerStop", domain::TravelTimePolicy::MINUTES_PER_STOP},
                                           {"note", domain::TravelTimePolicy::assumptions()}};
                       return platform::HttpResponse::ok(body);
                   });
        router.post(
            "/road-nodes",
            [this](const platform::RequestContext& context)
            {
                auto input = platform::InputValidator::body(context.request);
                application::AddRoadNodeRequest request;
                request.id = input.requiredText("id", 1, MAX_NODE_ID);
                request.label = input.requiredText("label", 1, MAX_LABEL);
                request.zone = input.code("zone");
                request.x = input.requiredNumber("x", MIN_COORDINATE, MAX_COORDINATE);
                request.y = input.requiredNumber("y", MIN_COORDINATE, MAX_COORDINATE);
                for (const auto& [destinationId, costKm] :
                     input.textNumberPairs("links", "to", MAX_NODE_ID, "costKm", MAX_COORDINATE))
                {
                    request.links.pushBack(domain::RoadLink {destinationId, costKm});
                }
                input.validate();
                const auto& node = m_addNode.execute(context.user(), request);
                return platform::HttpResponse::created(RoutingJsonPresenter::node(node),
                                                       locationOf("/road-nodes/" + node.id));
            },
            application::AddRoadNodeUseCase::IDEMPOTENCY);
        router.patch("/road-nodes/:id",
                     [this](const platform::RequestContext& context)
                     {
                         auto path = platform::InputValidator::parameters(context);
                         const std::string id {path.requiredText("id", 1, MAX_NODE_ID)};
                         path.validate();
                         auto input = platform::InputValidator::body(context.request);
                         application::UpdateRoadNodeRequest request;
                         if (input.has("zone"))
                         {
                             request.zone = input.code("zone");
                         }
                         request.active = input.optionalFlag("active");
                         input.validate();
                         return platform::HttpResponse::ok(
                             RoutingJsonPresenter::node(m_updateNode.execute(context.user(), id, request)));
                     });
        router.get(
            "/route-previews",
            [this](const platform::RequestContext& context)
            {
                auto input = platform::InputValidator::parameters(context);
                const std::string from {input.requiredText("from", 1, MAX_NODE_ID)};
                const std::string to {input.requiredText("to", 1, MAX_NODE_ID)};
                const auto algorithm =
                    input.optionalTerm<domain::SearchAlgorithm>("algorithm", Vocabulary::searchAlgorithm, ALGORITHMS)
                        .value_or(domain::SearchAlgorithm::AStar);
                input.validate();
                const auto trace = m_preview.execute(context.user(), from, to, algorithm);
                return platform::HttpResponse::ok(RoutingJsonPresenter::searchTrace(trace, m_getRoadMap.execute()));
            });
    }

} // namespace hubflow::routing::adapters
