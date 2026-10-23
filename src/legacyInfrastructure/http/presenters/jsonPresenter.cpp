#include "infrastructure/http/presenters/jsonPresenter.hpp"

#include "application/shared/vocabulary.hpp"

#include <cmath>

namespace hubflow::infrastructure
{
namespace
{
using application::Vocabulary;
using domain::logistics::Shipment;

constexpr double ROUNDING {100.0};

double rounded(const double value)
{
    return std::round(value * ROUNDING) / ROUNDING;
}

Json nodeIds(const ds::DynamicArray<std::size_t>& indexes, const domain::routing::RoadMap& map)
{
    Json ids = Json::array();
    for (std::size_t i = 0; i < indexes.size(); ++i)
    {
        ids.push_back(map.node(indexes[i]).id);
    }
    return ids;
}

Json searchMetrics(const domain::routing::SearchMetrics& metrics)
{
    return Json {{"solution_found", metrics.solutionFound},
                 {"nodes_explored", metrics.nodesExplored},
                 {"states_visited", metrics.statesVisited},
                 {"nodes_in_frontier", metrics.nodesInFrontier},
                 {"max_depth", metrics.maxDepth},
                 {"cost_total", rounded(metrics.costTotal)}};
}

Json bstNode(const domain::logistics::ShipmentBst::Node* pNode)
{
    Json node {nullptr};
    if (pNode != nullptr)
    {
        node = Json {{"code", pNode->key}, {"left", bstNode(pNode->pLeft)}, {"right", bstNode(pNode->pRight)}};
    }
    return node;
}

Json bPlusNode(const domain::logistics::ShipmentBPlusTree::Node* pNode)
{
    Json node {nullptr};
    if (pNode != nullptr)
    {
        Json keys     = Json::array();
        Json children = Json::array();
        for (std::size_t i = 0; i < pNode->keyCount; ++i)
        {
            keys.push_back(pNode->keys[i]);
        }
        for (std::size_t i = 0; !pNode->isLeaf && i <= pNode->keyCount; ++i)
        {
            children.push_back(bPlusNode(pNode->pChildren[i]));
        }
        node = Json {{"leaf", pNode->isLeaf}, {"keys", keys}, {"children", children}};
    }
    return node;
}

Json bits(const domain::logistics::StateHistory& history)
{
    Json states = Json::array();
    for (const auto status : domain::logistics::ALL_STATUSES)
    {
        if (history.test(domain::logistics::bitOf(status)))
        {
            states.push_back(Vocabulary::name(status));
        }
    }
    return states;
}

std::string binary(const domain::logistics::StateHistory& history)
{
    std::string text;
    for (std::size_t position = domain::logistics::STATUS_COUNT; position > 0; --position)
    {
        text.push_back(history.test(position - 1) ? '1' : '0');
    }
    return text;
}
} // namespace

Json JsonPresenter::user(const domain::identity::UserState& user)
{
    return Json {{"id", user.id},
                 {"email", user.email},
                 {"name", user.name},
                 {"role", Vocabulary::name(user.role)},
                 {"active", user.active}};
}

Json JsonPresenter::session(const application::Session& session)
{
    return Json {{"token", session.token}, {"expiresAt", session.expiresAt}, {"user", user(session.user)}};
}

Json JsonPresenter::shipmentSummary(const Shipment& shipment)
{
    return Json {{"code", shipment.code()},
                 {"recipient", shipment.recipient()},
                 {"zone", Vocabulary::name(shipment.zone())},
                 {"weightKg", rounded(shipment.weightKg())},
                 {"service", Vocabulary::name(shipment.service())},
                 {"status", Vocabulary::name(shipment.status())},
                 {"attempts", shipment.attempts()},
                 {"deliveryPoint", shipment.deliveryPoint()}};
}

Json JsonPresenter::shipment(const Shipment& shipment)
{
    Json body             = shipmentSummary(shipment);
    body["stateHistory"]  = stateHistory(shipment.stateHistory());
    body["movementCount"] = shipment.history().size();
    body["lastMovement"] =
        shipment.history().latest() == nullptr ? Json {nullptr} : movement(*shipment.history().latest());
    return body;
}

Json JsonPresenter::movement(const domain::logistics::Movement& movement)
{
    return Json {{"sequence", movement.sequence},
                 {"status", Vocabulary::name(movement.status)},
                 {"note", movement.note},
                 {"actor", movement.actorName},
                 {"occurredAt", movement.occurredAt}};
}

Json JsonPresenter::history(const Shipment& shipment, const bool newestFirst)
{
    Json       items   = Json::array();
    const auto collect = [&items](const domain::logistics::Movement& entry) { items.push_back(movement(entry)); };
    if (newestFirst)
    {
        shipment.history().forEachReverse(collect);
    }
    else
    {
        shipment.history().forEachChronological(collect);
    }
    return Json {{"code", shipment.code()}, {"order", newestFirst ? "desc" : "asc"}, {"items", items}};
}

Json JsonPresenter::stateHistory(const domain::logistics::StateHistory& history)
{
    return Json {{"bits", binary(history)},
                 {"value", history.raw()},
                 {"states", bits(history)},
                 {"count", history.count()},
                 {"any", history.any()},
                 {"everRescheduled", history.test(domain::logistics::bitOf(domain::logistics::ShipmentStatus::Reprogramado))}};
}

Json JsonPresenter::zoneSummary(const domain::logistics::Zone zone, const domain::logistics::ZoneSummary& summary)
{
    return Json {{"zone", Vocabulary::name(zone)},
                 {"count", summary.count},
                 {"totalWeightKg", rounded(summary.totalWeightKg)},
                 {"expressCount", summary.expressCount}};
}

Json JsonPresenter::metrics(const application::OperationsMetrics& metrics)
{
    Json byStatus = Json::object();
    for (const auto status : domain::logistics::ALL_STATUSES)
    {
        byStatus[Vocabulary::name(status)] = metrics.byStatus[domain::logistics::bitOf(status)];
    }
    Json        zones = Json::array();
    std::size_t index {0};
    for (const auto zone : domain::logistics::ALL_ZONES)
    {
        zones.push_back(zoneSummary(zone, metrics.pendingByZone[index++]));
    }
    return Json {{"totalShipments", metrics.totalShipments},
                 {"byStatus", byStatus},
                 {"totalAttempts", metrics.totalAttempts},
                 {"pendingByZone", zones}};
}

Json JsonPresenter::sortingBatch(const domain::sorting::SortingBatch& batch)
{
    Json items       = Json::array();
    Json preparation = Json::array();
    Json processed   = Json::array();
    for (std::size_t i = 0; i < batch.size(); ++i)
    {
        items.push_back(shipmentSummary(batch.at(i)));
    }
    batch.preparation().forEach([&preparation](const Shipment* pShipment) { preparation.push_back(pShipment->code()); });
    batch.processed().forEach([&processed](const Shipment* pShipment) { processed.push_back(pShipment->code()); });
    return Json {{"id", batch.id()},
                 {"zone", Vocabulary::name(batch.zone())},
                 {"size", batch.size()},
                 {"sortedBy", Vocabulary::name(batch.sortedBy())},
                 {"items", items},
                 {"preparationQueue", preparation},
                 {"processedStack", processed}};
}

Json JsonPresenter::sortStats(const domain::sorting::SortStats& stats)
{
    return Json {{"algorithm", "INSERTION_SORT"}, {"comparisons", stats.comparisons}, {"shifts", stats.shifts}};
}

Json JsonPresenter::batchSearch(const domain::sorting::BatchSearchOutcome& outcome,
                                const domain::sorting::SortingBatch&       batch)
{
    Json steps = Json::array();
    outcome.steps.forEach(
        [&steps](const domain::sorting::BinarySearchStep& step)
        {
            steps.push_back(Json {
                {"left", step.left}, {"right", step.right}, {"middle", step.middle}, {"codeAtMiddle", step.codeAtMiddle}});
        });
    return Json {{"found", outcome.position.has_value()},
                 {"position", outcome.position.has_value() ? Json(*outcome.position) : Json(nullptr)},
                 {"shipment", outcome.position.has_value() ? shipmentSummary(batch.at(*outcome.position)) : Json(nullptr)},
                 {"comparisons", outcome.comparisons},
                 {"steps", steps}};
}

Json JsonPresenter::bst(const domain::logistics::ShipmentBst& tree)
{
    Json inOrder = Json::array();
    tree.inOrder([&inOrder](const std::string& code, const Shipment*) { inOrder.push_back(code); });
    return Json {{"size", tree.size()}, {"height", tree.height()}, {"inOrder", inOrder}, {"root", bstNode(tree.root())}};
}

Json JsonPresenter::bstLookup(const application::BstLookup& lookup)
{
    Json path = Json::array();
    for (std::size_t i = 0; i < lookup.path.size(); ++i)
    {
        const auto& visit     = lookup.path[i];
        const char* direction = visit.step == ds::BstStep::Left ? "izquierda" : visit.step == ds::BstStep::Right ? "derecha" : "encontrado";
        path.push_back(Json {{"code", visit.code}, {"direction", direction}});
    }
    return Json {{"found", lookup.pShipment != nullptr},
                 {"visited", lookup.visited},
                 {"path", path},
                 {"shipment", lookup.pShipment == nullptr ? Json(nullptr) : shipment(*lookup.pShipment)}};
}

Json JsonPresenter::bPlusTree(const domain::logistics::ShipmentBPlusTree& tree)
{
    Json leaves = Json::array();
    tree.forEachInOrder([&leaves](const std::string& code, const Shipment*) { leaves.push_back(code); });
    return Json {{"order", 4},
                 {"size", tree.size()},
                 {"levels", tree.levels()},
                 {"leavesAtSameDepth", tree.leavesAtSameDepth()},
                 {"sequence", leaves},
                 {"root", bPlusNode(tree.root())}};
}

Json JsonPresenter::codeSet(const domain::insights::CodeSet& set)
{
    Json codes = Json::array();
    set.forEach([&codes](const std::string& code) { codes.push_back(code); });
    return Json {{"size", set.size()}, {"codes", codes}};
}

Json JsonPresenter::stateComparison(const domain::insights::StateComparison& comparison)
{
    return Json {{"common", stateHistory(comparison.common)},
                 {"onlyFirst", stateHistory(comparison.onlyFirst)},
                 {"onlySecond", stateHistory(comparison.onlySecond)},
                 {"either", stateHistory(comparison.either)},
                 {"exclusive", stateHistory(comparison.exclusive)}};
}

Json JsonPresenter::shipments(const ds::DynamicArray<const Shipment*>& shipments)
{
    Json items = Json::array();
    for (std::size_t i = 0; i < shipments.size(); ++i)
    {
        items.push_back(shipmentSummary(*shipments[i]));
    }
    return items;
}

Json JsonPresenter::roadMap(const domain::routing::RoadMap& map)
{
    Json nodes = Json::array();
    Json edges = Json::array();
    for (std::size_t i = 0; i < map.nodeCount(); ++i)
    {
        const auto& node = map.node(i);
        nodes.push_back(Json {{"id", node.id},
                              {"label", node.label},
                              {"zone", Vocabulary::name(node.zone)},
                              {"isHub", node.isHub},
                              {"x", node.x},
                              {"y", node.y}});
    }
    map.graph().forEachEdge(
        [&edges, &map](const std::size_t from, const std::size_t to, const double cost)
        { edges.push_back(Json {{"from", map.node(from).id}, {"to", map.node(to).id}, {"costKm", rounded(cost)}}); });
    return Json {{"nodes", nodes}, {"edges", edges}};
}

Json JsonPresenter::searchTrace(const domain::routing::SearchTrace& trace, const domain::routing::RoadMap& map)
{
    Json steps = Json::array();
    trace.steps.forEach(
        [&steps, &map](const domain::routing::TraceStep& step)
        {
            steps.push_back(Json {{"step", step.step},
                                  {"expanded", map.node(step.expanded).id},
                                  {"frontier", nodeIds(step.frontier, map)},
                                  {"explored", nodeIds(step.explored, map)}});
        });
    return Json {{"algorithm", Vocabulary::name(trace.algorithm)},
                 {"from", map.node(trace.start).id},
                 {"to", map.node(trace.goal).id},
                 {"steps", steps},
                 {"path", nodeIds(trace.path, map)},
                 {"metrics", searchMetrics(trace.metrics)}};
}

Json JsonPresenter::routeSummary(const domain::routing::DeliveryRoute& route)
{
    std::size_t resolved {0};
    for (std::size_t i = 0; i < route.stops().size(); ++i)
    {
        resolved += route.stops()[i].outcome == domain::routing::StopOutcome::Pending ? 0 : 1;
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

Json JsonPresenter::route(const domain::routing::DeliveryRoute& route, const domain::routing::RoadMap& map)
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

Json JsonPresenter::initialState(const domain::routing::DeliveryRoute& route, const domain::routing::RoadMap& map)
{
    Json goals = Json::array();
    for (std::size_t i = 0; i < route.stops().size(); ++i)
    {
        goals.push_back(route.stops()[i].nodeId);
    }
    Json state       = roadMap(map);
    state["start"]   = map.node(map.hub()).id;
    state["goals"]   = goals;
    state["courier"] = route.courier().name;
    state["routeId"] = route.id();
    return state;
}

Json JsonPresenter::sequence(const domain::routing::DeliveryRoute& route, const domain::routing::RoadMap& map)
{
    Json segments = Json::array();
    Json fullPath = Json::array();
    route.forEachSegment(
        [&](const domain::routing::SearchTrace& trace)
        {
            const Json segment = searchTrace(trace, map);
            for (const auto& node : segment.at("path"))
            {
                if (fullPath.empty() || fullPath.back() != node)
                {
                    fullPath.push_back(node);
                }
            }
            segments.push_back(segment);
        });
    return Json {{"algorithm", Vocabulary::name(route.algorithm())},
                 {"segments", segments},
                 {"path", fullPath},
                 {"metrics", Json {{"cost_total", rounded(route.totalCostKm())}, {"nodes_explored", route.totalNodesExplored()}}}};
}

Json JsonPresenter::auditEntry(const application::AuditEntry& entry)
{
    return Json {{"id", entry.id},
                 {"occurredAt", entry.occurredAt},
                 {"actor", Json {{"id", entry.actorId}, {"name", entry.actorName}}},
                 {"type", entry.type},
                 {"subject", entry.subject},
                 {"detail", entry.detail}};
}

} // namespace hubflow::infrastructure
