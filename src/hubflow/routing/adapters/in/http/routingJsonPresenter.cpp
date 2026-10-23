#include "hubflow/routing/adapters/in/http/routingJsonPresenter.hpp"

#include "hubflow/routing/adapters/out/export/routePlanJson.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

#include <cmath>

namespace hubflow::routing::adapters
{
namespace
{
using platform::Json;
using shared::application::Vocabulary;

constexpr double ROUNDING {100.0};

double rounded(const double value)
{
    return std::round(value * ROUNDING) / ROUNDING;
}
} // namespace

Json RoutingJsonPresenter::roadMap(const domain::RoadMap& map)
{
    return RoutePlanJson::roadMap(map);
}

Json RoutingJsonPresenter::searchTrace(const domain::SearchTrace& trace, const domain::RoadMap& map)
{
    return RoutePlanJson::searchTrace(map, trace);
}

Json RoutingJsonPresenter::routeSummary(const domain::DeliveryRoute& route)
{
    std::size_t resolved {0};
    for (std::size_t i = 0; i < route.stops().size(); ++i)
    {
        resolved += route.stops()[i].outcome == domain::StopOutcome::Pending ? 0 : 1;
    }
    return Json {{"id", route.id()},
                 {"courier", Json {{"id", route.courier().id}, {"name", route.courier().name}}},
                 {"algorithm", Vocabulary::name(route.algorithm())},
                 {"status", Vocabulary::name(route.status())},
                 {"createdAt", route.createdAt()},
                 {"stopCount", route.stops().size()},
                 {"resolvedStops", resolved},
                 {"totalCostKm", rounded(route.totalCostKm())},
                 {"nodesExplored", route.totalNodesExplored()}};
}

Json RoutingJsonPresenter::route(const domain::DeliveryRoute& route, const domain::RoadMap& map)
{
    Json stops = Json::array();
    for (std::size_t i = 0; i < route.stops().size(); ++i)
    {
        const auto& stop = route.stops()[i];
        stops.push_back(Json {{"order", stop.order},
                              {"shipmentCode", stop.shipmentCode},
                              {"nodeId", stop.nodeId},
                              {"outcome", Vocabulary::name(stop.outcome)}});
    }
    Json body        = routeSummary(route);
    body["stops"]    = stops;
    body["sequence"] = sequence(route, map);
    return body;
}

Json RoutingJsonPresenter::initialState(const domain::DeliveryRoute& route, const domain::RoadMap& map)
{
    return RoutePlanJson::initialState(map, route);
}

Json RoutingJsonPresenter::sequence(const domain::DeliveryRoute& route, const domain::RoadMap& map)
{
    return RoutePlanJson::sequence(map, route);
}

} // namespace hubflow::routing::adapters
