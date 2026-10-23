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
    const auto batchId = http.app.openSortingBatch.execute(http.app.operador(), logistics::domain::Zone::Norte).id();
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
    const json  invalid {{"code", "XYZ"}, {"recipient", "A"}, {"zone", "OESTE"}, {"weightKg", -3}};
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
