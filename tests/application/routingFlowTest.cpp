#include "support/applicationHarness.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using shared::domain::business_error;

namespace
{
std::uint64_t createCourier(test::ApplicationHarness& app, const std::string& email)
{
    return app.registerUser
        .execute(app.admin(),
                 identity::application::RegisterUserRequest {
                     email, "Repartidor", identity::domain::Role::Pum, "clave-segura"})
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
} // namespace

TEST(RoutingFlow, AssigningRouteDispatchesLoadInLifoOrder)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto courierId = createCourier(app, "pum1@hubflow.local");

    const auto& route = assignNorthRoute(app, courierId);

    EXPECT_EQ(route.stops().size(), 3U);
    EXPECT_EQ(route.stops()[0].shipmentCode, "PKG-1008");
    EXPECT_EQ(route.stops()[2].shipmentCode, "PKG-1002");
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1005").status(),
              logistics::domain::ShipmentStatus::EnReparto);
    EXPECT_EQ(app.summarizeZone.execute(app.operador(), "NORTE").count, 0U);
    EXPECT_EQ(storage.routes.records.size(), 1U);
    EXPECT_EQ(app.getActiveSortingBatch.execute(app.operador()), nullptr);
}

TEST(RoutingFlow, CourierResolvesOnlyOwnStops)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto  courierId = createCourier(app, "pum1@hubflow.local");
    const auto  otherId   = createCourier(app, "pum2@hubflow.local");
    const auto& route     = assignNorthRoute(app, courierId);

    EXPECT_THROW(app.resolveRouteStop.execute(app.pum(otherId),
                                              routing::application::ResolveStopRequest {
                                                  route.id(), 1, routing::domain::StopOutcome::Delivered, "", true}),
                 business_error);
    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});
    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 2, routing::domain::StopOutcome::Rescheduled, ""});

    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1008").status(),
              logistics::domain::ShipmentStatus::Entregado);
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1005").attempts(), 1U);
    EXPECT_EQ(app.listRoutes.execute(app.pum(otherId)).size(), 0U);
    EXPECT_EQ(storage.routes.records.at(route.id()).stops[0].outcome, routing::domain::StopOutcome::Delivered);
}

TEST(RoutingFlow, HydrationRebuildsModelFromStorage)
{
    test::Storage storage;
    std::uint64_t routeId {0};
    {
        test::ApplicationHarness app {storage};
        app.seedDataset();
        const auto courierId = createCourier(app, "pum1@hubflow.local");
        routeId              = assignNorthRoute(app, courierId).id();
        app.resolveRouteStop.execute(
            app.pum(courierId),
            routing::application::ResolveStopRequest {routeId, 2, routing::domain::StopOutcome::Rescheduled, ""});
    }
    test::ApplicationHarness restored {storage};
    std::vector<std::string> pending;
    restored.listPendingShipments.execute(restored.operador())
        .forEach(
            [&pending](const auto& shipment)
            {
                pending.push_back(shipment.code());
            });

    EXPECT_EQ(pending,
              (std::vector<std::string> {"PKG-1006", "PKG-1003", "PKG-1005", "PKG-1001", "PKG-1004", "PKG-1007"}));
    EXPECT_EQ(restored.getShipment.execute(restored.operador(), "PKG-1005").history().size(), 3U);
    EXPECT_EQ(restored.getRoute.execute(restored.admin(), routeId).stops()[1].outcome,
              routing::domain::StopOutcome::Rescheduled);
    EXPECT_EQ(restored.logistics.catalog.bPlusTree().size(), 8U);
}

TEST(RoutingFlow, AssignmentRequiresProcessedLoadAndActiveCourier)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto  courierId = createCourier(app, "pum1@hubflow.local");
    const auto& batch     = app.openSortingBatch.execute(app.operador(), "SUR");
    EXPECT_THROW(app.assignDeliveryRoute.execute(app.operador(),
                                                 routing::application::AssignRouteRequest {batch.id(), courierId, {}}),
                 business_error);
    app.prepareSortingBatch.execute(app.operador(), batch.id());
    app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
    EXPECT_THROW(
        app.assignDeliveryRoute.execute(app.operador(), routing::application::AssignRouteRequest {batch.id(), 999, {}}),
        business_error);
}

TEST(RoutingFlow, ResolvingAStopWithTheSameOutcomeChangesNothing)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto                                     courierId = createCourier(app, "pum1@hubflow.local");
    const auto&                                    route     = assignNorthRoute(app, courierId);
    const routing::application::ResolveStopRequest delivered {
        route.id(), 1, routing::domain::StopOutcome::Delivered, "", true};

    app.resolveRouteStop.execute(app.pum(courierId), delivered);
    const auto auditedEvents = storage.audit.entries.size();
    const auto historySize   = app.getShipment.execute(app.operador(), "PKG-1008").history().size();
    app.resolveRouteStop.execute(app.pum(courierId), delivered);

    EXPECT_EQ(storage.audit.entries.size(), auditedEvents);
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1008").history().size(), historySize);
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1008").status(),
              logistics::domain::ShipmentStatus::Entregado);
}

TEST(RoutingFlow, ResolvingAStopWithADifferentOutcomeIsAConflict)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto  courierId = createCourier(app, "pum1@hubflow.local");
    const auto& route     = assignNorthRoute(app, courierId);
    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});

    shared::domain::ErrorKind kind {shared::domain::ErrorKind::Validation};
    try
    {
        app.resolveRouteStop.execute(
            app.pum(courierId),
            routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Rescheduled, ""});
    }
    catch (const business_error& error)
    {
        kind = error.kind();
    }

    EXPECT_EQ(kind, shared::domain::ErrorKind::Conflict);
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1008").status(),
              logistics::domain::ShipmentStatus::Entregado);
}
