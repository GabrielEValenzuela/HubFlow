#include "support/fixtures.hpp"
#include "support/httpHarness.hpp"

#include <gtest/gtest.h>
#include <string>

using namespace hubflow;
using nlohmann::json;
using test::HttpHarness;

namespace
{
    constexpr const char* JSON_TYPE {"application/json"};

    std::string trackingUrl(const std::string& code)
    {
        return "/api/v1/public/tracking/" + code;
    }

    httplib::Result verify(HttpHarness& http, const std::string& code, const json& body)
    {
        return http.client().Post(trackingUrl(code) + "/verifications", body.dump(), JSON_TYPE);
    }

    httplib::Headers bearer(const std::string& token)
    {
        return httplib::Headers {{"Authorization", "Bearer " + token}};
    }

    std::string tokenFor(HttpHarness& http, const std::string& code, const json& body)
    {
        return HttpHarness::bodyOf(verify(http, code, body)).at("token").get<std::string>();
    }

    std::uint64_t routeForNorth(HttpHarness& http)
    {
        const auto batchId = http.app.openSortingBatch.execute(http.app.operador(), "NORTE").id();
        http.app.sortSortingBatch.execute(http.app.operador(), batchId, sorting::domain::SortCriteria::CodeAscending);
        http.app.prepareSortingBatch.execute(http.app.operador(), batchId);
        for (int i = 0; i < 3; ++i)
        {
            http.app.processSortingBatchItem.execute(http.app.operador(), batchId, {});
        }
        return http.app.assignDeliveryRoute
            .execute(
                http.app.operador(),
                routing::application::AssignRouteRequest {batchId, http.pumId, routing::domain::SearchAlgorithm::AStar})
            .id();
    }
} // namespace

TEST(PublicTrackingApi, AnyoneWithTheCodeSeesTheJourneyWithoutLoggingIn)
{
    HttpHarness http;
    http.app.seedDataset();

    const auto result = http.client().Get(trackingUrl("PKG-1002"));

    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, 200);
    const auto body = HttpHarness::bodyOf(result);
    EXPECT_EQ(body.at("code"), "PKG-1002");
    EXPECT_EQ(body.at("status").at("stage"), "RECEIVED");
    EXPECT_EQ(body.at("status").at("label"), "Recibimos tu paquete en el centro de distribución");
    ASSERT_EQ(body.at("timeline").size(), 1U);
    EXPECT_TRUE(body.at("timeline")[0].at("at").is_number());
    EXPECT_EQ(result->get_header_value("Cache-Control"), "no-store");
}

TEST(PublicTrackingApi, ThePublicViewNeverLeaksInternalOrPersonalData)
{
    HttpHarness http;
    http.app.seedDataset();
    http.app.classifyShipment.execute(http.app.operador(), "PKG-1002", "NOTA-INTERNA-SECRETA");

    const auto result = http.client().Get(trackingUrl("PKG-1002"));

    ASSERT_TRUE(result);
    for (const char* pForbidden : {"NOTA-INTERNA-SECRETA",
                                   "Operador",
                                   "Bruno",
                                   "Díaz",
                                   "5532",
                                   "4410",
                                   "1640",
                                   "N2",
                                   "Parque Norte",
                                   "recipient",
                                   "actor"})
    {
        EXPECT_EQ(result->body.find(pForbidden), std::string::npos) << pForbidden;
    }
}

TEST(PublicTrackingApi, BothCodeFormatsAreAcceptedInAnyCase)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto& hf = http.app.registerShipment.execute(http.app.operador(), test::draft("", "EXPRESS", "NORTE", 1.0));
    http.app.registerShipment.execute(http.app.operador(), test::draft("PKG-4000", "EXPRESS", "NORTE", 1.0));

    EXPECT_EQ(http.client().Get(trackingUrl("pkg-4000"))->status, 200);
    EXPECT_EQ(http.client().Get(trackingUrl(hf.code()))->status, 200);
    std::string lower {hf.code()};
    for (auto& character : lower)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    EXPECT_EQ(http.client().Get(trackingUrl(lower))->status, 200);
}

TEST(PublicTrackingApi, UnknownCodeAndWrongProofAnswerTheSameProblem)
{
    HttpHarness http;
    http.app.seedDataset();

    const auto unknown = http.client().Get(trackingUrl("PKG-9999"));
    const auto malformed = http.client().Get(trackingUrl("hola"));
    const auto unknownVerification = verify(http, "PKG-9999", json {{"phoneLast4", "1234"}});
    const auto wrongProof = verify(http, "PKG-1002", json {{"phoneLast4", "1234"}});

    for (const auto* pResult : {&malformed, &unknownVerification, &wrongProof})
    {
        ASSERT_TRUE(*pResult);
        EXPECT_EQ((*pResult)->status, 404);
        EXPECT_EQ((*pResult)->body, unknown->body);
        EXPECT_EQ((*pResult)->get_header_value("Content-Type"), unknown->get_header_value("Content-Type"));
    }
    EXPECT_EQ(HttpHarness::bodyOf(unknown).at("code"), "tracking.not_found");
}

TEST(PublicTrackingApi, VerifyingGrantsAShortLivedTokenForTheExtendedView)
{
    HttpHarness http;
    http.app.seedDataset();

    const auto verification = verify(http, "PKG-1002", json {{"phoneLast4", "4410"}});

    ASSERT_TRUE(verification);
    EXPECT_EQ(verification->status, 200);
    const auto grant = HttpHarness::bodyOf(verification);
    EXPECT_TRUE(grant.at("expiresAt").is_number());
    const auto extended = http.client().Get(trackingUrl("PKG-1002"), bearer(grant.at("token")));
    ASSERT_TRUE(extended);
    EXPECT_EQ(extended->status, 200);
    const auto body = HttpHarness::bodyOf(extended);
    EXPECT_EQ(body.at("details").at("recipientFirstName"), "Bruno");
    EXPECT_EQ(body.at("details").at("zone"), "Norte");
    EXPECT_EQ(body.at("details").at("neighbourhood"), "Parque Norte");
    EXPECT_EQ(body.at("details").at("parcels").at("total"), 1);
    EXPECT_EQ(body.at("details").at("parcels").at("delivered"), 0);
    EXPECT_TRUE(body.at("details").at("estimatedArrival").is_null());
    for (const char* pForbidden : {"5532", "4410", "1640", "Díaz", "N2", "actor", "note"})
    {
        EXPECT_EQ(extended->body.find(pForbidden), std::string::npos) << pForbidden;
    }
}

TEST(PublicTrackingApi, TheExtendedViewNeedsAValidTokenForThatSameCode)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto token = tokenFor(http, "PKG-1002", json {{"postalCode", "1640"}});

    const auto none = http.client().Get(trackingUrl("PKG-1002"));
    const auto otherCode = http.client().Get(trackingUrl("PKG-1003"), bearer(token));
    const auto garbage = http.client().Get(trackingUrl("PKG-1002"), bearer("basura"));
    const auto unknownCode = http.client().Get(trackingUrl("PKG-9999"), bearer(token));
    const auto staffToken = http.client().Get(trackingUrl("PKG-1002"), http.as("admin@hubflow.local"));

    EXPECT_EQ(none->status, 200);
    EXPECT_EQ(none->body.find("details"), std::string::npos);
    for (const auto* pResult : {&otherCode, &garbage, &unknownCode, &staffToken})
    {
        EXPECT_EQ((*pResult)->status, 401);
        EXPECT_EQ((*pResult)->body, otherCode->body);
        EXPECT_EQ((*pResult)->body.find("details"), std::string::npos);
    }
}

TEST(PublicTrackingApi, AnExpiredTokenIsRefused)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto token = tokenFor(http, "PKG-1002", json {{"phoneLast4", "4410"}});

    http.app.clock.advance(15 * 60);

    EXPECT_EQ(http.client().Get(trackingUrl("PKG-1002"), bearer(token))->status, 401);
}

TEST(PublicTrackingApi, VerificationRequiresSomeProofInTheRightShape)
{
    HttpHarness http;
    http.app.seedDataset();

    const auto empty = verify(http, "PKG-1002", json::object());
    const auto badLast = verify(http, "PKG-1002", json {{"phoneLast4", "12345"}});

    EXPECT_EQ(empty->status, 422);
    EXPECT_EQ(badLast->status, 422);
}

TEST(PublicTrackingApi, TooManyAttemptsAreAnswered429WithRetryAfter)
{
    HttpHarness http;
    http.app.seedDataset();
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(verify(http, "PKG-1002", json {{"phoneLast4", "0000"}})->status, 404);
    }

    const auto blocked = verify(http, "PKG-1002", json {{"phoneLast4", "4410"}});

    ASSERT_TRUE(blocked);
    EXPECT_EQ(blocked->status, 429);
    EXPECT_EQ(blocked->get_header_value("Retry-After"), "900");
    EXPECT_EQ(HttpHarness::bodyOf(blocked).at("code"), "tracking.too_many_attempts");
    EXPECT_EQ(verify(http, "PKG-1003", json {{"phoneLast4", "3391"}})->status, 200);
}

TEST(PublicTrackingApi, UnknownCodesAreRateLimitedExactlyLikeKnownOnes)
{
    HttpHarness http;
    http.app.seedDataset();
    for (int i = 0; i < 5; ++i)
    {
        verify(http, "PKG-9999", json {{"phoneLast4", "0000"}});
    }

    EXPECT_EQ(verify(http, "PKG-9999", json {{"phoneLast4", "0000"}})->status, 429);
}

TEST(PublicTrackingApi, OnTheWayThePackageShowsAnEstimatedWindowAndThenDelivered)
{
    HttpHarness http;
    http.app.seedDataset();
    const auto routeId = routeForNorth(http);
    const auto token = tokenFor(http, "PKG-1002", json {{"phoneLast4", "4410"}});

    const auto onTheWay = HttpHarness::bodyOf(http.client().Get(trackingUrl("PKG-1002"), bearer(token)));

    EXPECT_EQ(onTheWay.at("status").at("label"), "Está en camino");
    const auto& arrival = onTheWay.at("details").at("estimatedArrival");
    ASSERT_TRUE(arrival.is_object());
    EXPECT_EQ(arrival.at("stopsBefore"), 2);
    EXPECT_EQ(arrival.at("to").get<std::int64_t>() - arrival.at("from").get<std::int64_t>(), 3600);

    for (std::size_t order = 1; order <= 3; ++order)
    {
        http.app.resolveRouteStop.execute(http.app.pum(http.pumId),
                                          routing::application::ResolveStopRequest {
                                              routeId, order, routing::domain::StopOutcome::Delivered, "", true});
    }
    const auto delivered = HttpHarness::bodyOf(http.client().Get(trackingUrl("PKG-1002"), bearer(token)));
    EXPECT_EQ(delivered.at("status").at("label"), "Entregado");
    EXPECT_TRUE(delivered.at("finished").get<bool>());
    EXPECT_TRUE(delivered.at("details").at("estimatedArrival").is_null());
    EXPECT_EQ(delivered.at("timeline").size(), 3U);
}

TEST(PublicTrackingApi, SettingsExposeTheTrackingLinkBase)
{
    HttpHarness http;

    const auto result = http.client().Get("/api/v1/public/tracking-settings");

    ASSERT_TRUE(result);
    EXPECT_EQ(HttpHarness::bodyOf(result).at("trackingLinkBase"), "http://localhost:8480/seguimiento/");
}

TEST(PublicTrackingApi, StaffEndpointsStillNeedASession)
{
    HttpHarness http;
    http.app.seedDataset();

    EXPECT_EQ(http.client().Get("/api/v1/shipments")->status, 401);
    EXPECT_EQ(http.client().Get("/api/v1/customer-notices")->status, 401);
}

TEST(PublicTrackingApi, OnlyTheAdministratorSeesTheCustomerNotices)
{
    HttpHarness http;
    http.app.seedDataset();

    const auto asAdmin = http.client().Get("/api/v1/customer-notices?limit=2", http.as("admin@hubflow.local"));
    const auto asOperator = http.client().Get("/api/v1/customer-notices", http.as("operador@hubflow.local"));

    ASSERT_TRUE(asAdmin);
    EXPECT_EQ(asAdmin->status, 200);
    const auto body = HttpHarness::bodyOf(asAdmin);
    EXPECT_EQ(body.at("total"), 8);
    ASSERT_EQ(body.at("items").size(), 2U);
    EXPECT_EQ(body.at("items")[0].at("shipmentCode"), "PKG-1008");
    EXPECT_EQ(body.at("items")[0].at("recipient"), "****5521");
    EXPECT_EQ(asOperator->status, 403);
}

TEST(PublicTrackingApi, RegisteringRequiresPhoneAndPostalCodeInAValidFormat)
{
    HttpHarness http;
    json shipment {
        {"recipient", "Marta Lopez"},
        {"recipientPhone", "123"},
        {"recipientPostalCode", "zzz"},
        {"zone", "NORTE"},
        {"service", "EXPRESS"},
        {"deliveryPoint", "N2"},
        {"parcels", json::array({json {{"weightKg", 1.0}, {"lengthCm", 10}, {"widthCm", 10}, {"heightCm", 10}}})}};

    const auto invalid =
        http.client().Post("/api/v1/shipments", http.as("operador@hubflow.local"), shipment.dump(), JSON_TYPE);
    shipment["recipientPhone"] = "+54 11 4521-7788";
    shipment["recipientPostalCode"] = "c1425abc";
    const auto valid =
        http.client().Post("/api/v1/shipments", http.as("operador@hubflow.local"), shipment.dump(), JSON_TYPE);

    EXPECT_EQ(invalid->status, 422);
    const auto fields = HttpHarness::bodyOf(invalid).at("errors");
    EXPECT_EQ(fields.size(), 2U);
    ASSERT_EQ(valid->status, 201);
    EXPECT_EQ(HttpHarness::bodyOf(valid).at("recipientPhone"), "541145217788");
    EXPECT_EQ(HttpHarness::bodyOf(valid).at("recipientPostalCode"), "C1425ABC");
}
