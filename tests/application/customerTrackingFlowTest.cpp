#include "support/applicationHarness.hpp"

#include "hubflow/routing/domain/services/travelTimePolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using shared::domain::business_error;
using shared::domain::ErrorKind;
using tracking::application::VerificationRequest;

namespace
{
constexpr const char* HOME {"203.0.113.7"};

VerificationRequest withPhone(const std::string& code, const std::string& last4, const std::string& address = HOME)
{
    return VerificationRequest {address, code, last4, ""};
}

VerificationRequest
withPostalCode(const std::string& code, const std::string& postal, const std::string& address = HOME)
{
    return VerificationRequest {address, code, "", postal};
}

std::int64_t nowOf(const test::ApplicationHarness& app)
{
    const shared::application::Clock& clock = app.clock;
    return clock.now();
}

template <typename TAction>
business_error failureOf(TAction action)
{
    try
    {
        action();
    }
    catch (const business_error& error)
    {
        return error;
    }
    return business_error {ErrorKind::Validation, "none", "no falló"};
}

std::uint64_t createCourier(test::ApplicationHarness& app)
{
    return app.registerUser
        .execute(app.admin(),
                 identity::application::RegisterUserRequest {
                     "pum1@hubflow.local", "Repartidor", identity::domain::Role::Pum, "clave-segura"})
        .id;
}

const routing::domain::DeliveryRoute& assignNorthRoute(test::ApplicationHarness& app, const std::uint64_t courierId)
{
    const auto& batch = app.openSortingBatch.execute(app.operador(), "NORTE");
    app.sortSortingBatch.execute(app.operador(), batch.id(), sorting::domain::SortCriteria::CodeAscending);
    app.prepareSortingBatch.execute(app.operador(), batch.id());
    for (int i = 0; i < 3; ++i)
    {
        app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
    }
    return app.assignDeliveryRoute.execute(
        app.operador(),
        routing::application::AssignRouteRequest {batch.id(), courierId, routing::domain::SearchAlgorithm::AStar});
}

class Notices final : public tracking::application::CustomerNoticeVisitor
{
public:
    std::vector<tracking::application::CustomerNotice> items;

private:
    void visit(const tracking::application::StoredCustomerNotice& stored) override
    {
        items.push_back(stored.notice);
    }
};

std::vector<std::string> messagesFor(test::Storage& storage, const std::string& code)
{
    std::vector<std::string> messages;
    for (const auto& notice : storage.notices.notices)
    {
        if (notice.shipmentCode == code)
        {
            messages.push_back(notice.message);
        }
    }
    return messages;
}

bool anyContains(const std::vector<std::string>& messages, const std::string& text)
{
    return std::any_of(messages.begin(),
                       messages.end(),
                       [&text](const std::string& message)
                       {
                           return message.find(text) != std::string::npos;
                       });
}
} // namespace

TEST(CustomerTracking, PublicViewTellsTheJourneyInCustomerLanguage)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    app.classifyShipment.execute(app.operador(), "PKG-1002", "Nota interna: el cliente es conflictivo");

    const auto view = app.getPublicTracking.execute("PKG-1002");

    EXPECT_EQ(view.code, "PKG-1002");
    ASSERT_EQ(view.timeline.size(), 2U);
    EXPECT_EQ(view.timeline[0].text, "Recibimos tu paquete en el centro de distribución");
    EXPECT_EQ(view.timeline[1].text, "Tu paquete fue preparado para el reparto");
    EXPECT_EQ(view.current.text, "Tu paquete fue preparado para el reparto");
    EXPECT_FALSE(view.finished);
    EXPECT_EQ(view.timeline[0].occurredAt, nowOf(app));
}

TEST(CustomerTracking, CodesAreAcceptedInAnyCaseAndWithSpaces)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    EXPECT_EQ(app.getPublicTracking.execute(" pkg-1002 ").code, "PKG-1002");
}

TEST(CustomerTracking, UnknownMalformedAndFailedVerificationAreIndistinguishable)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    const auto unknown = failureOf(
        [&]
        {
            app.getPublicTracking.execute("PKG-9999");
        });
    const auto malformed = failureOf(
        [&]
        {
            app.getPublicTracking.execute("hola");
        });
    const auto unknownVerification = failureOf(
        [&]
        {
            app.verifyTracking.execute(withPhone("PKG-9999", "4410"));
        });
    const auto wrongProof = failureOf(
        [&]
        {
            app.verifyTracking.execute(withPhone("PKG-1002", "0000"));
        });

    for (const auto* pError : {&malformed, &unknownVerification, &wrongProof})
    {
        EXPECT_EQ(pError->errorKind(), unknown.errorKind());
        EXPECT_EQ(pError->code(), unknown.code());
        EXPECT_STREQ(pError->what(), unknown.what());
    }
    EXPECT_EQ(unknown.errorKind(), ErrorKind::NotFound);
}

TEST(CustomerTracking, VerifyingWithThePhoneEndingGrantsTheExtendedView)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    const auto access = app.verifyTracking.execute(withPhone("PKG-1002", "4410"));
    const auto view   = app.getVerifiedTracking.execute("PKG-1002", access.token);

    EXPECT_EQ(access.expiresAt, nowOf(app) + 15 * 60);
    EXPECT_EQ(view.recipientFirstName, "Bruno");
    EXPECT_EQ(view.zoneName, "Norte");
    EXPECT_EQ(view.neighbourhood, "Parque Norte");
    EXPECT_EQ(view.parcelCount, 1U);
    EXPECT_EQ(view.parcelsDelivered, 0U);
    EXPECT_FALSE(view.arrival.has_value());
}

TEST(CustomerTracking, VerifyingWithThePostalCodeIgnoresCaseAndSpaces)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    const auto access = app.verifyTracking.execute(withPostalCode("PKG-1001", "c1425 abc"));

    EXPECT_EQ(app.getVerifiedTracking.execute("PKG-1001", access.token).recipientFirstName, "Ana");
}

TEST(CustomerTracking, EverythingInformedMustMatch)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    const auto error = failureOf(
        [&]
        {
            app.verifyTracking.execute(VerificationRequest {HOME, "PKG-1002", "4410", "9999"});
        });
    const auto both = app.verifyTracking.execute(VerificationRequest {HOME, "PKG-1002", "4410", "1640"});

    EXPECT_EQ(error.code(), "tracking.not_found");
    EXPECT_FALSE(both.token.empty());
}

TEST(CustomerTracking, VerifyingWithoutAnyProofIsAValidationError)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    EXPECT_EQ(failureOf(
                  [&]
                  {
                      app.verifyTracking.execute(withPhone("PKG-1002", ""));
                  })
                  .errorKind(),
              ErrorKind::Validation);
}

TEST(CustomerTracking, TheTokenOnlyOpensThePackageItWasIssuedFor)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto access = app.verifyTracking.execute(withPhone("PKG-1002", "4410"));

    const auto otherPackage = failureOf(
        [&]
        {
            app.getVerifiedTracking.execute("PKG-1003", access.token);
        });
    const auto garbage = failureOf(
        [&]
        {
            app.getVerifiedTracking.execute("PKG-1002", "basura");
        });
    const auto unknownCode = failureOf(
        [&]
        {
            app.getVerifiedTracking.execute("PKG-9999", access.token);
        });

    EXPECT_EQ(otherPackage.errorKind(), ErrorKind::Unauthorized);
    for (const auto* pError : {&garbage, &unknownCode})
    {
        EXPECT_EQ(pError->code(), otherPackage.code());
        EXPECT_STREQ(pError->what(), otherPackage.what());
    }
}

TEST(CustomerTracking, TheTokenExpiresAfterFifteenMinutes)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto access = app.verifyTracking.execute(withPhone("PKG-1002", "4410"));

    app.clock.advance(15 * 60 - 1);
    EXPECT_NO_THROW(app.getVerifiedTracking.execute("PKG-1002", access.token));
    app.clock.advance(1);
    EXPECT_EQ(failureOf(
                  [&]
                  {
                      app.getVerifiedTracking.execute("PKG-1002", access.token);
                  })
                  .errorKind(),
              ErrorKind::Unauthorized);
}

TEST(CustomerTracking, AttemptsAreLimitedPerAddressAndCode)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(failureOf(
                      [&]
                      {
                          app.verifyTracking.execute(withPhone("PKG-1002", "0000"));
                      })
                      .errorKind(),
                  ErrorKind::NotFound);
    }

    const auto blocked = failureOf(
        [&]
        {
            app.verifyTracking.execute(withPhone("PKG-1002", "4410"));
        });

    EXPECT_EQ(blocked.errorKind(), ErrorKind::RateLimited);
    EXPECT_EQ(blocked.retryAfterSeconds(), 15U * 60U);
    EXPECT_NO_THROW(app.verifyTracking.execute(withPhone("PKG-1002", "4410", "198.51.100.9")));
    EXPECT_NO_THROW(app.verifyTracking.execute(withPhone("PKG-1003", "3391")));

    app.clock.advance(15 * 60);
    EXPECT_NO_THROW(app.verifyTracking.execute(withPhone("PKG-1002", "4410")));
}

TEST(CustomerTracking, UnknownCodesAreLimitedTooSoTheLimitIsNoOracle)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(failureOf(
                      [&]
                      {
                          app.verifyTracking.execute(withPhone("PKG-9999", "0000"));
                      })
                      .errorKind(),
                  ErrorKind::NotFound);
    }

    EXPECT_EQ(failureOf(
                  [&]
                  {
                      app.verifyTracking.execute(withPhone("PKG-9999", "0000"));
                  })
                  .errorKind(),
              ErrorKind::RateLimited);
}

TEST(CustomerTracking, RetryAfterShrinksAsTheWindowMoves)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    for (int i = 0; i < 5; ++i)
    {
        failureOf(
            [&]
            {
                app.verifyTracking.execute(withPhone("PKG-9999", "0000"));
            });
    }
    app.clock.advance(60);

    const auto blocked = failureOf(
        [&]
        {
            app.verifyTracking.execute(withPhone("PKG-9999", "0000"));
        });

    EXPECT_EQ(blocked.errorKind(), ErrorKind::RateLimited);
    EXPECT_EQ(blocked.retryAfterSeconds(), 14U * 60U);
}

TEST(CustomerTracking, ExtendedViewShowsTheEstimatedWindowWhileOnTheWay)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto& route = assignNorthRoute(app, createCourier(app));
    ASSERT_EQ(route.stops()[2].shipmentCode, "PKG-1002");
    double      km {0.0};
    std::size_t index {0};
    route.forEachSegment(
        [&](const routing::domain::SearchTrace& trace)
        {
            km += index++ < 3 ? trace.metrics.costTotal : 0.0;
        });
    const auto expectedMinutes = routing::domain::TravelTimePolicy::estimateMinutes(km, 3);

    const auto access = app.verifyTracking.execute(withPhone("PKG-1002", "4410"));
    const auto view   = app.getVerifiedTracking.execute("PKG-1002", access.token);

    EXPECT_EQ(view.summary.current.text, "Está en camino");
    ASSERT_TRUE(view.arrival.has_value());
    EXPECT_EQ(view.arrival->stopsBefore, 2U);
    const auto arrival = nowOf(app) + static_cast<std::int64_t>(expectedMinutes) * 60;
    EXPECT_LE(view.arrival->window.from, arrival);
    EXPECT_GE(view.arrival->window.to, arrival);
    EXPECT_EQ(view.arrival->window.to - view.arrival->window.from, 60 * 60);
}

TEST(CustomerTracking, TheWindowShrinksAsStopsAreResolvedAndDisappearsOnDelivery)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto  courierId = createCourier(app);
    const auto& route     = assignNorthRoute(app, courierId);
    const auto  access    = app.verifyTracking.execute(withPhone("PKG-1002", "4410"));
    const auto  before    = app.getVerifiedTracking.execute("PKG-1002", access.token);

    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});
    const auto afterOne = app.getVerifiedTracking.execute("PKG-1002", access.token);
    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 2, routing::domain::StopOutcome::Delivered, "", true});
    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 3, routing::domain::StopOutcome::Delivered, "", true});
    const auto done = app.getVerifiedTracking.execute("PKG-1002", access.token);

    ASSERT_TRUE(before.arrival.has_value());
    ASSERT_TRUE(afterOne.arrival.has_value());
    EXPECT_EQ(afterOne.arrival->stopsBefore, 1U);
    EXPECT_LE(afterOne.arrival->window.from, before.arrival->window.from);
    EXPECT_TRUE(done.summary.finished);
    EXPECT_EQ(done.summary.current.text, "Entregado");
    EXPECT_FALSE(done.arrival.has_value());
    EXPECT_EQ(done.parcelsDelivered, 1U);
}

TEST(CustomerNotices, EveryStepOfTheJourneyProducesACustomerNotice)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto  courierId = createCourier(app);
    const auto& route     = assignNorthRoute(app, courierId);
    app.resolveRouteStop.execute(app.pum(courierId),
                                 routing::application::ResolveStopRequest {
                                     route.id(), 1, routing::domain::StopOutcome::Rescheduled, "ausente", false});
    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 3, routing::domain::StopOutcome::Delivered, "", true});

    const auto retried   = messagesFor(storage, "PKG-1008");
    const auto delivered = messagesFor(storage, "PKG-1002");

    ASSERT_EQ(retried.size(), 3U);
    EXPECT_NE(retried[0].find("Recibimos tu paquete en el centro de distribución"), std::string::npos);
    EXPECT_NE(retried[0].find("http://localhost:8480/seguimiento/?codigo=PKG-1008"), std::string::npos);
    EXPECT_EQ(retried[1], "Paquete PKG-1008: Está en camino");
    EXPECT_EQ(retried[2], "Paquete PKG-1008: No pudimos entregarlo, vamos a intentar de nuevo");
    EXPECT_EQ(delivered.back(), "Paquete PKG-1002: Entregado");
}

TEST(CustomerNotices, TheContactIsAlwaysMasked)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    for (const auto& notice : storage.notices.notices)
    {
        EXPECT_EQ(notice.recipient.substr(0, 4), "****");
        EXPECT_EQ(notice.recipient.size(), 8U);
        EXPECT_EQ(notice.channel, "SMS");
    }
    EXPECT_EQ(storage.notices.notices.size(), 8U);
    EXPECT_EQ(storage.notices.notices[1].recipient, "****4410");
}

TEST(CustomerNotices, PartialDeliveryExplainsHowManyParcelsArrived)
{
    test::Storage                    storage;
    test::ApplicationHarness         app {storage};
    logistics::domain::ShipmentDraft draft;
    draft.recipient           = "Marta Lopez";
    draft.recipientPhone      = "11 5555-8899";
    draft.recipientPostalCode = "1640";
    draft.zone                = "NORTE";
    draft.service             = "ESTANDAR";
    draft.deliveryPoint       = "N1";
    draft.parcels.pushBack(logistics::domain::ParcelData {1.0, 20.0, 20.0, 10.0});
    draft.parcels.pushBack(logistics::domain::ParcelData {2.0, 20.0, 20.0, 10.0});
    const auto  code      = app.registerShipment.execute(app.operador(), draft).code();
    const auto  courierId = createCourier(app);
    const auto& batch     = app.openSortingBatch.execute(app.operador(), "NORTE");
    app.prepareSortingBatch.execute(app.operador(), batch.id());
    app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
    const auto& route = app.assignDeliveryRoute.execute(
        app.operador(), routing::application::AssignRouteRequest {batch.id(), courierId, {}});
    app.confirmStopParcel.execute(app.pum(courierId),
                                  routing::application::ConfirmParcelRequest {route.id(), 1, code, 1});

    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Rescheduled, "", false});

    const auto messages = messagesFor(storage, code);
    EXPECT_EQ(messages.back(),
              "Paquete " + code + ": entregamos 1 de 2 bultos. El resto lo llevamos en el próximo intento");
    EXPECT_FALSE(anyContains({messages.back()}, "No pudimos entregarlo"));
}

TEST(CustomerNotices, OnlyAdministratorsListThem)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    Notices notices;

    const auto total  = app.listCustomerNotices.execute(app.admin(), 3, 0, notices);
    const auto forbid = failureOf(
        [&]
        {
            app.listCustomerNotices.execute(app.operador(), 3, 0, notices);
        });

    EXPECT_EQ(total, 8U);
    ASSERT_EQ(notices.items.size(), 3U);
    EXPECT_EQ(notices.items[0].shipmentCode, "PKG-1008");
    EXPECT_EQ(forbid.errorKind(), ErrorKind::Forbidden);
}
