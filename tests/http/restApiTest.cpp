#include "support/httpHarness.hpp"

#include <gtest/gtest.h>
#include <set>

using namespace hubflow;
using nlohmann::json;
using test::HttpHarness;

namespace
{
constexpr const char* JSON_TYPE {"application/json"};
constexpr const char* API {"/api/v1"};

std::string url(const std::string& path)
{
    return std::string {API} + path;
}

json validShipment(const std::string& code)
{
    return json {{"code", code},
                 {"recipient", "Marta Lopez"},
                 {"zone", "NORTE"},
                 {"weightKg", 2.5},
                 {"service", "EXPRESS"},
                 {"deliveryPoint", "N2"}};
}

std::uint64_t openPreparedNorthBatch(HttpHarness& http)
{
    const auto batchId = http.app.openSortingBatch.execute(http.app.operador(), "NORTE").id();
    http.app.sortSortingBatch.execute(http.app.operador(), batchId, sorting::domain::SortCriteria::CodeAscending);
    http.app.prepareSortingBatch.execute(http.app.operador(), batchId);
    return batchId;
}
} // namespace

TEST(RestApi, LoginReturnsTokenAndUser)
{
    HttpHarness http;
    const auto  result = http.client().Post(
        url("/sessions"), json {{"email", "operador@hubflow.local"}, {"password", "clave-segura"}}.dump(), JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 201);
    const auto body = HttpHarness::bodyOf(result);
    EXPECT_FALSE(body.at("token").get<std::string>().empty());
    EXPECT_EQ(body.at("user").at("role"), "OPERADOR");
    EXPECT_EQ(body.at("user").count("passwordHash"), 0U);
}

TEST(RestApi, WrongPasswordIsUnauthorizedProblemDetails)
{
    HttpHarness http;
    const auto  result = http.client().Post(
        url("/sessions"), json {{"email", "operador@hubflow.local"}, {"password", "equivocada"}}.dump(), JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 401);
    EXPECT_NE(result->get_header_value("Content-Type").find("application/problem+json"), std::string::npos);
}

TEST(RestApi, ProtectedEndpointWithoutTokenIsUnauthorized)
{
    HttpHarness http;
    const auto  result = http.client().Get(url("/shipments"));

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 401);
    EXPECT_EQ(HttpHarness::bodyOf(result).at("code"), "auth.missing_token");
}

TEST(RestApi, HealthIsPublicAndReportsDatabase)
{
    HttpHarness http;
    const auto  result = http.client().Get(url("/health"));

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 200);
    EXPECT_EQ(HttpHarness::bodyOf(result).at("database"), "ok");
}

TEST(RestApi, PumCannotRegisterShipments)
{
    HttpHarness http;
    const auto  result = http.client().Post(
        url("/shipments"), http.as("pum1@hubflow.local"), validShipment("PKG-2001").dump(), JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 403);
}

TEST(RestApi, InvalidShipmentListsEveryFieldError)
{
    HttpHarness http;
    const json  invalid {{"code", "XYZ"}, {"recipient", "A"}, {"zone", "oeste"}, {"weightKg", -3}};
    const auto  result =
        http.client().Post(url("/shipments"), http.as("operador@hubflow.local"), invalid.dump(), JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 422);
    const auto            body = HttpHarness::bodyOf(result);
    std::set<std::string> fields;
    for (const auto& error : body.at("errors"))
    {
        fields.insert(error.at("field").get<std::string>());
    }
    EXPECT_TRUE(fields.count("code") == 1U && fields.count("recipient") == 1U && fields.count("zone") == 1U);
    EXPECT_TRUE(fields.count("weightKg") == 1U && fields.count("service") == 1U && fields.count("deliveryPoint") == 1U);
}

TEST(RestApi, MalformedJsonIsBadRequest)
{
    HttpHarness http;
    const auto  result =
        http.client().Post(url("/shipments"), http.as("operador@hubflow.local"), "{no es json", JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 400);
}

TEST(RestApi, RegisteringShipmentRespondsCreatedWithLocation)
{
    HttpHarness http;
    const auto  result = http.client().Post(
        url("/shipments"), http.as("operador@hubflow.local"), validShipment("PKG-2001").dump(), JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 201);
    EXPECT_EQ(result->get_header_value("Location"), "/api/v1/shipments/PKG-2001");
    EXPECT_EQ(HttpHarness::bodyOf(result).at("status"), "RECIBIDO");
}

TEST(RestApi, DuplicateShipmentCodeIsConflict)
{
    HttpHarness http;
    const auto  headers = http.as("operador@hubflow.local");
    http.client().Post(url("/shipments"), headers, validShipment("PKG-2001").dump(), JSON_TYPE);
    const auto second = http.client().Post(url("/shipments"), headers, validShipment("PKG-2001").dump(), JSON_TYPE);

    ASSERT_TRUE(second);
    EXPECT_EQ(second->status, 409);
}

TEST(RestApi, IdempotencyKeyReplaysDispatchWithoutDispatchingTwice)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto headers       = http.as("operador@hubflow.local", {{"Idempotency-Key", "dispatch-1"}});
    const auto pendingBefore = http.app.listPendingShipments.execute(http.app.operador()).size();

    const auto first  = http.client().Post(url("/dispatches"), headers, json::object().dump(), JSON_TYPE);
    const auto second = http.client().Post(url("/dispatches"), headers, json::object().dump(), JSON_TYPE);

    ASSERT_TRUE(first && second);
    EXPECT_EQ(first->status, 201);
    EXPECT_EQ(second->status, 201);
    EXPECT_TRUE(first->get_header_value("Idempotent-Replayed").empty());
    EXPECT_EQ(second->get_header_value("Idempotent-Replayed"), "true");
    EXPECT_EQ(second->get_header_value("Location"), first->get_header_value("Location"));
    EXPECT_EQ(HttpHarness::bodyOf(second), HttpHarness::bodyOf(first));
    EXPECT_EQ(http.app.listPendingShipments.execute(http.app.operador()).size(), pendingBefore - 1);
}

TEST(RestApi, DispatchWithoutKeyExecutesEveryTime)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto headers = http.as("operador@hubflow.local");
    const auto first   = http.client().Post(url("/dispatches"), headers, json::object().dump(), JSON_TYPE);
    const auto second  = http.client().Post(url("/dispatches"), headers, json::object().dump(), JSON_TYPE);

    ASSERT_TRUE(first && second);
    EXPECT_NE(HttpHarness::bodyOf(second).at("code"), HttpHarness::bodyOf(first).at("code"));
}

TEST(RestApi, ReusingKeyOnAnotherRouteIsUnprocessable)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto headers = http.as("operador@hubflow.local", {{"Idempotency-Key", "shared-key"}});
    http.client().Post(url("/dispatches"), headers, json::object().dump(), JSON_TYPE);

    const auto other = http.client().Post(url("/shipments"), headers, validShipment("PKG-2001").dump(), JSON_TYPE);

    ASSERT_TRUE(other);
    EXPECT_EQ(other->status, 422);
}

TEST(RestApi, OverlongIdempotencyKeyIsUnprocessable)
{
    HttpHarness http;
    const auto  headers = http.as("operador@hubflow.local", {{"Idempotency-Key", std::string(81, 'k')}});
    const auto  result  = http.client().Post(url("/shipments"), headers, validShipment("PKG-2001").dump(), JSON_TYPE);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 422);
}

TEST(RestApi, ResolvingTheSameStopTwiceRespondsOkBothTimes)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto batchId = openPreparedNorthBatch(http);
    http.app.processSortingBatchItem.execute(http.app.operador(), batchId);
    const auto routeId = http.app.assignDeliveryRoute
                             .execute(http.app.operador(),
                                      routing::application::AssignRouteRequest {
                                          batchId, http.pumId, routing::domain::SearchAlgorithm::AStar})
                             .id();
    const auto headers = http.as("pum1@hubflow.local");
    const auto path    = url("/routes/" + std::to_string(routeId) + "/stops/1");
    const json body {{"outcome", "ENTREGADO"}, {"note", "Recibió el titular"}};

    const auto first  = http.client().Patch(path, headers, body.dump(), JSON_TYPE);
    const auto second = http.client().Patch(path, headers, body.dump(), JSON_TYPE);
    const auto other  = http.client().Patch(path, headers, json {{"outcome", "REPROGRAMADO"}}.dump(), JSON_TYPE);

    ASSERT_TRUE(first && second && other);
    EXPECT_EQ(first->status, 200);
    EXPECT_EQ(second->status, 200);
    EXPECT_EQ(HttpHarness::bodyOf(second).at("stops")[0].at("outcome"), "ENTREGADO");
    EXPECT_EQ(other->status, 409);
}

TEST(RestApi, RouteSequenceIsAnAttachmentWithSegments)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto batchId = openPreparedNorthBatch(http);
    http.app.processSortingBatchItem.execute(http.app.operador(), batchId);
    const auto routeId = http.app.assignDeliveryRoute
                             .execute(http.app.operador(),
                                      routing::application::AssignRouteRequest {
                                          batchId, http.pumId, routing::domain::SearchAlgorithm::AStar})
                             .id();
    const auto headers = http.as("pum1@hubflow.local");

    const auto sequence = http.client().Get(url("/routes/" + std::to_string(routeId) + "/sequence"), headers);
    const auto initial  = http.client().Get(url("/routes/" + std::to_string(routeId) + "/initial-state"), headers);

    ASSERT_TRUE(sequence && initial);
    EXPECT_EQ(sequence->status, 200);
    EXPECT_NE(sequence->get_header_value("Content-Disposition").find("sequence.json"), std::string::npos);
    const auto body = HttpHarness::bodyOf(sequence);
    EXPECT_EQ(body.at("algorithm"), "ASTAR");
    EXPECT_TRUE(body.at("segments").is_array() && !body.at("segments").empty());
    EXPECT_TRUE(body.at("segments")[0].contains("steps") && body.at("segments")[0].contains("metrics"));
    EXPECT_EQ(body.at("path")[0], "HUB");
    EXPECT_NE(initial->get_header_value("Content-Disposition").find("initial_state.json"), std::string::npos);
    EXPECT_EQ(HttpHarness::bodyOf(initial).at("start"), "HUB");
}

TEST(RestApi, CourierSeesRecipientsOnOwnRouteAndOthersAreForbidden)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto batchId = openPreparedNorthBatch(http);
    http.app.processSortingBatchItem.execute(http.app.operador(), batchId);
    const auto routeId = http.app.assignDeliveryRoute
                             .execute(http.app.operador(),
                                      routing::application::AssignRouteRequest {
                                          batchId, http.pumId, routing::domain::SearchAlgorithm::AStar})
                             .id();
    const auto path = url("/routes/" + std::to_string(routeId));

    const auto own   = http.client().Get(path, http.as("pum1@hubflow.local"));
    const auto other = http.client().Get(path, http.as("pum2@hubflow.local"));

    ASSERT_TRUE(own && other);
    EXPECT_EQ(own->status, 200);
    const auto stop = HttpHarness::bodyOf(own).at("stops")[0];
    EXPECT_FALSE(stop.at("recipient").get<std::string>().empty());
    EXPECT_TRUE(stop.contains("service") && stop.contains("zone") && stop.contains("weightKg"));
    EXPECT_EQ(other->status, 403);

    const auto resolved = http.client().Patch(
        path + "/stops/1", http.as("pum1@hubflow.local"), json {{"outcome", "ENTREGADO"}}.dump(), JSON_TYPE);
    ASSERT_TRUE(resolved);
    EXPECT_EQ(resolved->status, 200);
    const auto resolvedStop = HttpHarness::bodyOf(resolved).at("stops")[0];
    EXPECT_EQ(resolvedStop.at("recipient"), stop.at("recipient"));
    EXPECT_TRUE(resolvedStop.contains("weightKg"));
}

TEST(RestApi, AnotherPumCannotReadTheRoute)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto batchId = openPreparedNorthBatch(http);
    http.app.processSortingBatchItem.execute(http.app.operador(), batchId);
    const auto routeId = http.app.assignDeliveryRoute
                             .execute(http.app.operador(),
                                      routing::application::AssignRouteRequest {
                                          batchId, http.pumId, routing::domain::SearchAlgorithm::AStar})
                             .id();

    const auto result =
        http.client().Get(url("/routes/" + std::to_string(routeId) + "/sequence"), http.as("pum2@hubflow.local"));

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 403);
}

TEST(RestApi, PreparingTwiceAfterLoadingDoesNotQueueLoadedPackagesAgain)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto batchId = openPreparedNorthBatch(http);
    http.app.processSortingBatchItem.execute(http.app.operador(), batchId);
    const auto headers = http.as("operador@hubflow.local");
    const auto path    = url("/sorting-batches/" + std::to_string(batchId) + "/preparation-queue");

    const auto first  = http.client().Put(path, headers, "", JSON_TYPE);
    const auto second = http.client().Put(path, headers, "", JSON_TYPE);

    ASSERT_TRUE(first && second);
    EXPECT_EQ(first->status, 200);
    EXPECT_EQ(second->status, 200);
    const auto body = HttpHarness::bodyOf(second);
    EXPECT_EQ(body.at("processedStack").size(), 1U);
    EXPECT_EQ(body.at("preparationQueue").size() + 1U, body.at("size").get<std::size_t>());
    EXPECT_EQ(HttpHarness::bodyOf(first).at("preparationQueue"), body.at("preparationQueue"));
}

TEST(RestApi, DeletingProcessedItemTwiceIsNotFoundTheSecondTime)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto        batchId = openPreparedNorthBatch(http);
    const std::string code {http.app.processSortingBatchItem.execute(http.app.operador(), batchId).code()};
    const auto        headers = http.as("operador@hubflow.local");
    const auto        path    = url("/sorting-batches/" + std::to_string(batchId) + "/processed-items/" + code);

    const auto first  = http.client().Delete(path, headers);
    const auto second = http.client().Delete(path, headers);

    ASSERT_TRUE(first && second);
    EXPECT_EQ(first->status, 200);
    EXPECT_EQ(HttpHarness::bodyOf(first).at("shipment").at("code"), code);
    EXPECT_EQ(second->status, 404);
}

TEST(RestApi, RoadMapAndRoutePreviewAreQueries)
{
    HttpHarness http;
    const auto  headers = http.as("operador@hubflow.local");

    const auto map     = http.client().Get(url("/road-map"), headers);
    const auto preview = http.client().Get(url("/route-previews?from=HUB&to=N2&algorithm=ASTAR"), headers);

    ASSERT_TRUE(map && preview);
    EXPECT_EQ(map->status, 200);
    EXPECT_FALSE(HttpHarness::bodyOf(map).at("nodes").empty());
    EXPECT_EQ(preview->status, 200);
    EXPECT_EQ(HttpHarness::bodyOf(preview).at("path").back(), "N2");
}

TEST(RestApi, UnknownApiRouteIsProblemDetails)
{
    HttpHarness http;
    const auto  result = http.client().Get(url("/nope"));

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 404);
    EXPECT_NE(result->get_header_value("Content-Type").find("application/problem+json"), std::string::npos);
}

TEST(RestApi, AdministratorConfiguresZonesAndOthersCannot)
{
    HttpHarness http;
    const json  oeste {{"code", "OESTE"}, {"name", "Oeste"}};

    const auto denied   = http.client().Post(url("/zones"), http.as("operador@hubflow.local"), oeste.dump(), JSON_TYPE);
    const auto created  = http.client().Post(url("/zones"), http.as("admin@hubflow.local"), oeste.dump(), JSON_TYPE);
    const auto repeated = http.client().Post(url("/zones"), http.as("admin@hubflow.local"), oeste.dump(), JSON_TYPE);
    const auto malformed = http.client().Post(
        url("/zones"), http.as("admin@hubflow.local"), json {{"code", "oeste"}, {"name", "x"}}.dump(), JSON_TYPE);

    ASSERT_TRUE(denied && created && repeated && malformed);
    EXPECT_EQ(denied->status, 403);
    EXPECT_EQ(created->status, 201);
    EXPECT_EQ(created->get_header_value("Location"), url("/zones/OESTE"));
    EXPECT_EQ(repeated->status, 409);
    EXPECT_EQ(malformed->status, 422);

    const auto listed = http.client().Get(url("/zones"), http.as("pum1@hubflow.local"));
    ASSERT_TRUE(listed);
    EXPECT_EQ(listed->status, 200);
    const auto items = HttpHarness::bodyOf(listed).at("items");
    ASSERT_EQ(items.size(), 4U);
    EXPECT_EQ(items[3].at("code"), "OESTE");
    EXPECT_TRUE(items[3].at("active").get<bool>());
}

TEST(RestApi, PatchingAZoneIsIdempotentAndDeactivationRespectsTheRules)
{
    HttpHarness http;
    http.app.seedDataset();
    const json close {{"active", false}};
    const auto headers = http.as("admin@hubflow.local");

    const auto blocked = http.client().Patch(url("/zones/NORTE"), headers, close.dump(), JSON_TYPE);
    ASSERT_TRUE(blocked);
    EXPECT_EQ(blocked->status, 409);
    EXPECT_EQ(HttpHarness::bodyOf(blocked).at("code"), "zone.has_shipments");

    http.client().Post(url("/zones"), headers, json {{"code", "OESTE"}, {"name", "Oeste"}}.dump(), JSON_TYPE);
    const auto first  = http.client().Patch(url("/zones/OESTE"), headers, close.dump(), JSON_TYPE);
    const auto second = http.client().Patch(url("/zones/OESTE"), headers, close.dump(), JSON_TYPE);
    const auto rename =
        http.client().Patch(url("/zones/OESTE"), headers, json {{"name", "Zona Oeste"}}.dump(), JSON_TYPE);
    const auto unknown = http.client().Patch(url("/zones/ESTE"), headers, close.dump(), JSON_TYPE);

    ASSERT_TRUE(first && second && rename && unknown);
    EXPECT_EQ(first->status, 200);
    EXPECT_EQ(second->status, 200);
    EXPECT_EQ(HttpHarness::bodyOf(first), HttpHarness::bodyOf(second));
    EXPECT_FALSE(HttpHarness::bodyOf(rename).at("active").get<bool>());
    EXPECT_EQ(HttpHarness::bodyOf(rename).at("name"), "Zona Oeste");
    EXPECT_EQ(unknown->status, 404);
}

TEST(RestApi, ANewZoneWithMapPointsReceivesPackagesWithFeatures)
{
    HttpHarness http;
    const auto  admin = http.as("admin@hubflow.local");
    http.client().Post(url("/zones"), admin, json {{"code", "OESTE"}, {"name", "Oeste"}}.dump(), JSON_TYPE);
    const json point {{"id", "O1"},
                      {"label", "Plaza Oeste"},
                      {"zone", "OESTE"},
                      {"x", 6.0},
                      {"y", 10.0},
                      {"links", json::array({json {{"to", "HUB"}, {"costKm", 4.5}}})}};

    const auto createdPoint  = http.client().Post(url("/road-nodes"), admin, point.dump(), JSON_TYPE);
    const auto repeatedPoint = http.client().Post(url("/road-nodes"), admin, point.dump(), JSON_TYPE);
    const auto tooShort      = http.client().Post(url("/road-nodes"),
                                             admin,
                                             json {{"id", "O2"},
                                                        {"label", "Atajo"},
                                                        {"zone", "OESTE"},
                                                        {"x", 0.0},
                                                        {"y", 10.0},
                                                        {"links", json::array({json {{"to", "HUB"}, {"costKm", 1.0}}})}}
                                                 .dump(),
                                             JSON_TYPE);
    const auto noLinks =
        http.client().Post(url("/road-nodes"),
                           admin,
                           json {{"id", "O3"}, {"label", "Isla"}, {"zone", "OESTE"}, {"x", 1.0}, {"y", 1.0}}.dump(),
                           JSON_TYPE);

    ASSERT_TRUE(createdPoint && repeatedPoint && tooShort && noLinks);
    EXPECT_EQ(createdPoint->status, 201);
    EXPECT_EQ(HttpHarness::bodyOf(createdPoint).at("zone"), "OESTE");
    EXPECT_EQ(repeatedPoint->status, 409);
    EXPECT_EQ(tooShort->status, 422);
    EXPECT_EQ(noLinks->status, 422);

    json shipment             = validShipment("PKG-3001");
    shipment["zone"]          = "OESTE";
    shipment["deliveryPoint"] = "O1";
    shipment["features"]      = json::array({"FRAGIL", "REFRIGERADO"});
    const auto registered =
        http.client().Post(url("/shipments"), http.as("operador@hubflow.local"), shipment.dump(), JSON_TYPE);
    ASSERT_TRUE(registered);
    EXPECT_EQ(registered->status, 201);
    EXPECT_EQ(HttpHarness::bodyOf(registered).at("features"), json::array({"FRAGIL", "REFRIGERADO"}));

    shipment["code"]     = "PKG-3002";
    shipment["features"] = json::array({"NO_EXISTE"});
    const auto badFeature =
        http.client().Post(url("/shipments"), http.as("operador@hubflow.local"), shipment.dump(), JSON_TYPE);
    ASSERT_TRUE(badFeature);
    EXPECT_EQ(badFeature->status, 422);

    const auto summary = http.client().Get(url("/zones/OESTE/summary"), http.as("operador@hubflow.local"));
    ASSERT_TRUE(summary);
    EXPECT_EQ(HttpHarness::bodyOf(summary).at("count"), 1);
    const auto missing = http.client().Get(url("/zones/ESTE/summary"), http.as("operador@hubflow.local"));
    ASSERT_TRUE(missing);
    EXPECT_EQ(missing->status, 404);

    const auto map = http.client().Get(url("/road-map"), http.as("operador@hubflow.local"));
    ASSERT_TRUE(map);
    const auto mapBody = HttpHarness::bodyOf(map);
    bool       found {false};
    for (const auto& node : mapBody.at("nodes"))
    {
        found = found || (node.at("id") == "O1" && node.at("zone") == "OESTE" && node.at("active").get<bool>());
    }
    EXPECT_TRUE(found);
}

TEST(RestApi, ServiceLevelsAndFeaturesAreManagedAsData)
{
    HttpHarness http;
    const auto  admin = http.as("admin@hubflow.local");

    const auto service = http.client().Post(url("/service-levels"),
                                            admin,
                                            json {{"code", "RELAMPAGO"}, {"name", "Relámpago"}, {"priority", 9}}.dump(),
                                            JSON_TYPE);
    const auto feature = http.client().Post(
        url("/shipment-features"), admin, json {{"code", "DOCUMENTOS"}, {"name", "Documentos"}}.dump(), JSON_TYPE);
    const auto badPriority = http.client().Post(
        url("/service-levels"), admin, json {{"code", "NADA"}, {"name", "Nada"}, {"priority", 0}}.dump(), JSON_TYPE);

    ASSERT_TRUE(service && feature && badPriority);
    EXPECT_EQ(service->status, 201);
    EXPECT_EQ(feature->status, 201);
    EXPECT_EQ(badPriority->status, 422);

    http.client().Post(
        url("/shipments"), http.as("operador@hubflow.local"), validShipment("PKG-3001").dump(), JSON_TYPE);
    json urgent       = validShipment("PKG-3002");
    urgent["service"] = "RELAMPAGO";
    http.client().Post(url("/shipments"), http.as("operador@hubflow.local"), urgent.dump(), JSON_TYPE);
    const auto pending = http.client().Get(url("/pending-shipments"), http.as("operador@hubflow.local"));
    ASSERT_TRUE(pending);
    EXPECT_EQ(HttpHarness::bodyOf(pending).at("items")[0].at("code"), "PKG-3002");

    const auto services = http.client().Get(url("/service-levels"), http.as("operador@hubflow.local"));
    const auto features = http.client().Get(url("/shipment-features"), http.as("operador@hubflow.local"));
    ASSERT_TRUE(services && features);
    EXPECT_EQ(HttpHarness::bodyOf(services).at("items").size(), 4U);
    EXPECT_EQ(HttpHarness::bodyOf(features).at("items").size(), 4U);

    const auto blocked =
        http.client().Patch(url("/service-levels/RELAMPAGO"), admin, json {{"active", false}}.dump(), JSON_TYPE);
    ASSERT_TRUE(blocked);
    EXPECT_EQ(blocked->status, 409);
}

TEST(RestApi, ACourierWithARouteInProgressIsDeactivatedOnlyAfterTheRouteIsReassigned)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto batchId = openPreparedNorthBatch(http);
    http.app.processSortingBatchItem.execute(http.app.operador(), batchId);
    const auto routeId = http.app.assignDeliveryRoute
                             .execute(http.app.operador(),
                                      routing::application::AssignRouteRequest {
                                          batchId, http.pumId, routing::domain::SearchAlgorithm::AStar})
                             .id();
    const auto admin = http.as("admin@hubflow.local");
    const auto close = json {{"active", false}}.dump();

    const auto blocked = http.client().Patch(url("/users/" + std::to_string(http.pumId)), admin, close, JSON_TYPE);
    ASSERT_TRUE(blocked);
    EXPECT_EQ(blocked->status, 409);
    EXPECT_EQ(HttpHarness::bodyOf(blocked).at("code"), "user.has_route_in_progress");

    const auto path      = url("/routes/" + std::to_string(routeId) + "/courier");
    const auto body      = json {{"courierId", http.otherPumId}}.dump();
    const auto byCourier = http.client().Put(path, http.as("pum1@hubflow.local"), body, JSON_TYPE);
    const auto moved     = http.client().Put(path, http.as("operador@hubflow.local"), body, JSON_TYPE);
    const auto again     = http.client().Put(path, admin, body, JSON_TYPE);
    const auto invalid   = http.client().Put(path, admin, json {{"courierId", 9999}}.dump(), JSON_TYPE);

    ASSERT_TRUE(byCourier && moved && again && invalid);
    EXPECT_EQ(byCourier->status, 403);
    EXPECT_EQ(moved->status, 200);
    EXPECT_EQ(HttpHarness::bodyOf(moved).at("courier").at("id"), http.otherPumId);
    EXPECT_EQ(again->status, 200);
    EXPECT_EQ(invalid->status, 422);

    const auto closed = http.client().Patch(url("/users/" + std::to_string(http.pumId)), admin, close, JSON_TYPE);
    ASSERT_TRUE(closed);
    EXPECT_EQ(closed->status, 200);
    EXPECT_FALSE(HttpHarness::bodyOf(closed).at("active").get<bool>());

    const auto audit = http.client().Get(url("/audit-events"), admin);
    ASSERT_TRUE(audit);
    const auto auditBody = HttpHarness::bodyOf(audit);
    bool       reassigned {false};
    for (const auto& event : auditBody.at("items"))
    {
        reassigned = reassigned || event.at("type") == "route.reassigned";
    }
    EXPECT_TRUE(reassigned);
}
