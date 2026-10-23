#include "hubflow/routing/adapters/out/export/routePlanJson.hpp"

#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::routing::adapters
{
namespace
{
using nlohmann::json;
using shared::application::Vocabulary;

json idsOf(const domain::RoadMap& map, const ds::DynamicArray<std::size_t>& indexes)
{
    json ids = json::array();
    for (std::size_t i = 0; i < indexes.size(); ++i)
    {
        ids.push_back(map.node(indexes[i]).id);
    }
    return ids;
}

json stepsOf(const domain::RoadMap& map, const domain::SearchTrace& trace)
{
    json steps = json::array();
    trace.steps.forEach(
        [&map, &steps](const domain::TraceStep& step)
        {
            steps.push_back({{"step", step.step},
                             {"expanded", map.node(step.expanded).id},
                             {"frontier", idsOf(map, step.frontier)},
                             {"explored", idsOf(map, step.explored)}});
        });
    return steps;
}

json metricsOf(const domain::SearchMetrics& metrics)
{
    return {{"solution_found", metrics.solutionFound},
            {"nodes_explored", metrics.nodesExplored},
            {"states_visited", metrics.statesVisited},
            {"nodes_in_frontier", metrics.nodesInFrontier},
            {"max_depth", metrics.maxDepth},
            {"cost_total", metrics.costTotal}};
}

json segmentOf(const domain::RoadMap& map, const domain::SearchTrace& trace)
{
    return {{"from", map.node(trace.start).id},
            {"to", map.node(trace.goal).id},
            {"steps", stepsOf(map, trace)},
            {"path", idsOf(map, trace.path)},
            {"metrics", metricsOf(trace.metrics)}};
}

json nodesOf(const domain::RoadMap& map)
{
    json nodes = json::array();
    for (std::size_t i = 0; i < map.nodeCount(); ++i)
    {
        const auto& node = map.node(i);
        nodes.push_back({{"id", node.id},
                         {"label", node.label},
                         {"zone", node.zone},
                         {"active", node.active},
                         {"isHub", node.isHub},
                         {"x", node.x},
                         {"y", node.y}});
    }
    return nodes;
}

json edgesOf(const domain::RoadMap& map)
{
    json edges = json::array();
    map.graph().forEachEdge(
        [&map, &edges](const std::size_t from, const std::size_t to, const double costKm)
        {
            edges.push_back({{"from", map.node(from).id}, {"to", map.node(to).id}, {"costKm", costKm}});
        });
    return edges;
}

json goalsOf(const domain::DeliveryRoute& route)
{
    json goals = json::array();
    for (std::size_t i = 0; i < route.stops().size(); ++i)
    {
        goals.push_back(route.stops()[i].nodeId);
    }
    return goals;
}
} // namespace

json RoutePlanJson::initialState(const domain::RoadMap& map, const domain::DeliveryRoute& route)
{
    return {{"routeId", route.id()},
            {"courier", route.courier().name},
            {"algorithm", Vocabulary::name(route.algorithm())},
            {"start", map.node(map.hub()).id},
            {"goals", goalsOf(route)},
            {"nodes", nodesOf(map)},
            {"edges", edgesOf(map)}};
}

json RoutePlanJson::sequence(const domain::RoadMap& map, const domain::DeliveryRoute& route)
{
    json segments = json::array();
    json fullPath = json::array();
    route.forEachSegment(
        [&map, &segments, &fullPath](const domain::SearchTrace& trace)
        {
            segments.push_back(segmentOf(map, trace));
            for (std::size_t i = fullPath.empty() ? 0 : 1; i < trace.path.size(); ++i)
            {
                fullPath.push_back(map.node(trace.path[i]).id);
            }
        });
    return {{"algorithm", Vocabulary::name(route.algorithm())},
            {"segments", segments},
            {"path", fullPath},
            {"metrics", {{"cost_total", route.totalCostKm()}, {"nodes_explored", route.totalNodesExplored()}}}};
}

json RoutePlanJson::roadMap(const domain::RoadMap& map)
{
    return {{"nodes", nodesOf(map)}, {"edges", edgesOf(map)}};
}

json RoutePlanJson::searchTrace(const domain::RoadMap& map, const domain::SearchTrace& trace)
{
    json body         = segmentOf(map, trace);
    body["algorithm"] = Vocabulary::name(trace.algorithm);
    return body;
}

} // namespace hubflow::routing::adapters
