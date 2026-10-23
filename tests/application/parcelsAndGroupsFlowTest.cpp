#include "support/applicationHarness.hpp"
#include "support/fixtures.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using logistics::domain::ParcelData;
using logistics::domain::ShipmentStatus;
using shared::domain::business_error;
using shared::domain::ErrorKind;

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

std::string registerAt(test::ApplicationHarness& app, const std::string& point, const std::size_t parcels)
{
    logistics::domain::ShipmentDraft shipment;
    shipment.recipient           = "Cliente";
    shipment.recipientPhone      = "11 5555-0000";
    shipment.recipientPostalCode = "1000";
    shipment.zone                = "NORTE";
    shipment.service             = "ESTANDAR";
    shipment.deliveryPoint       = point;
    for (std::size_t i = 0; i < parcels; ++i)
    {
        shipment.parcels.pushBack(ParcelData {1.0 + static_cast<double>(i), 20.0, 20.0, 10.0});
    }
    return app.registerShipment.execute(app.operador(), shipment).code();
}

ds::DynamicArray<std::string> listOf(const std::string& first, const std::string& second = "")
{
    ds::DynamicArray<std::string> list;
    list.pushBack(first);
    if (!second.empty())
    {
        list.pushBack(second);
    }
    return list;
}

ErrorKind kindOf(const std::function<void()>& action)
{
    ErrorKind kind {ErrorKind::Validation};
    try
    {
        action();
        kind = ErrorKind::Forbidden;
    }
    catch (const business_error& error)
    {
        kind = error.kind();
    }
    return kind;
}

struct Scenario
{
    explicit Scenario(test::Storage& storage)
        : app {storage}
    {
        first   = registerAt(app, "N2", 3);
        second  = registerAt(app, "N2", 1);
        alone   = registerAt(app, "N3", 2);
        courier = createCourier(app, "pum1@hubflow.local");
    }

    std::uint64_t openBatch()
    {
        const auto id = app.openSortingBatch.execute(app.operador(), "NORTE").id();
        app.sortSortingBatch.execute(app.operador(), id, sorting::domain::SortCriteria::CodeAscending);
        app.prepareSortingBatch.execute(app.operador(), id);
        return id;
    }

    const routing::domain::DeliveryRoute& routeOfTheGroup()
    {
        app.formShipmentGroup.execute(app.operador(), listOf(first, second));
        const auto batchId = openBatch();
        app.processSortingBatchItem.execute(app.operador(), batchId, listOf(first, second));
        return app.assignDeliveryRoute.execute(
            app.operador(),
            routing::application::AssignRouteRequest {batchId, courier, routing::domain::SearchAlgorithm::AStar});
    }

    test::ApplicationHarness app;
    std::string              first;
    std::string              second;
    std::string              alone;
    std::uint64_t            courier {0};
};
} // namespace

TEST(ParcelsAndGroupsFlow, RegistrationAssignsIdentityAndTrackingCodeAndKeepsParcels)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};

    const auto code = registerAt(app, "N2", 3);

    const auto& shipment = app.getShipment.execute(app.operador(), code);
    EXPECT_EQ(shipment.parcelCount(), 3U);
    EXPECT_TRUE(logistics::domain::ShipmentId::isValid(shipment.id()));
    EXPECT_EQ(shipment.parcel(2).label(), code + "-3/3");
    EXPECT_EQ(storage.shipments.states.at(code).draft.parcels.size(), 3U);
    EXPECT_EQ(storage.shipments.states.at(code).draft.id, shipment.id());
}

TEST(ParcelsAndGroupsFlow, TheDeskRefusesToLoadAGroupMemberWithoutTheRestOfItsGroup)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app = scenario.app;
    app.formShipmentGroup.execute(app.operador(), listOf(scenario.first, scenario.second));
    const auto batchId = scenario.openBatch();
    const auto queued  = app.getSortingBatch.execute(app.operador(), batchId).preparation().size();

    const auto kind = kindOf(
        [&]
        {
            app.processSortingBatchItem.execute(app.operador(), batchId, listOf(scenario.first));
        });

    EXPECT_EQ(kind, ErrorKind::Conflict);
    EXPECT_EQ(app.getSortingBatch.execute(app.operador(), batchId).preparation().size(), queued);
    EXPECT_TRUE(app.getSortingBatch.execute(app.operador(), batchId).processed().isEmpty());
    const auto loaded =
        app.processSortingBatchItem.execute(app.operador(), batchId, listOf(scenario.first, scenario.second));
    EXPECT_EQ(loaded.size(), 2U);
    EXPECT_EQ(app.getSortingBatch.execute(app.operador(), batchId).processed().size(), 2U);
}

TEST(ParcelsAndGroupsFlow, LoadingTheNextPackageBringsItsWholeGroup)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app = scenario.app;
    app.formShipmentGroup.execute(app.operador(), listOf(scenario.first, scenario.second));
    const auto batchId = scenario.openBatch();

    const auto loaded = app.processSortingBatchItem.execute(app.operador(), batchId, {});

    EXPECT_EQ(loaded.size(), 2U);
    EXPECT_EQ(loaded[0]->code(), scenario.first);
    EXPECT_EQ(loaded[1]->code(), scenario.second);
    const auto returned = app.returnProcessedItem.execute(app.operador(), batchId, scenario.second);
    EXPECT_EQ(returned.size(), 2U);
    EXPECT_TRUE(app.getSortingBatch.execute(app.operador(), batchId).processed().isEmpty());
}

TEST(ParcelsAndGroupsFlow, AGroupIsOneStopOnTheRoute)
{
    test::Storage storage;
    Scenario      scenario {storage};

    const auto& route = scenario.routeOfTheGroup();

    ASSERT_EQ(route.stops().size(), 1U);
    EXPECT_EQ(route.stops()[0].nodeId, "N2");
    EXPECT_EQ(route.stops()[0].shipmentCodes().size(), 2U);
    EXPECT_TRUE(route.stops()[0].includes(scenario.second));
    EXPECT_EQ(scenario.app.getShipment.execute(scenario.app.operador(), scenario.second).status(),
              ShipmentStatus::EnReparto);
    EXPECT_EQ(storage.routes.records.at(route.id()).stops[0].companions.size(), 1U);
}

TEST(ParcelsAndGroupsFlow, DeliveringTheStopNeedsEveryParcelOrAnExplicitDeliverAll)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app   = scenario.app;
    const auto&   route = scenario.routeOfTheGroup();
    const auto    pum   = app.pum(scenario.courier);

    const auto kind = kindOf(
        [&]
        {
            app.resolveRouteStop.execute(pum,
                                         routing::application::ResolveStopRequest {
                                             route.id(), 1, routing::domain::StopOutcome::Delivered, "", false});
        });

    EXPECT_EQ(kind, ErrorKind::Conflict);
    EXPECT_EQ(route.stops()[0].outcome, routing::domain::StopOutcome::Pending);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.first).status(), ShipmentStatus::EnReparto);
    app.resolveRouteStop.execute(
        pum,
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.first).status(), ShipmentStatus::Entregado);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.second).status(), ShipmentStatus::Entregado);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.first).pendingParcelCount(), 0U);
}

TEST(ParcelsAndGroupsFlow, ConfirmingEveryParcelAllowsClosingTheStopAsDelivered)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app   = scenario.app;
    const auto&   route = scenario.routeOfTheGroup();
    const auto    pum   = app.pum(scenario.courier);
    for (std::size_t ordinal = 1; ordinal <= 3; ++ordinal)
    {
        app.confirmStopParcel.execute(
            pum, routing::application::ConfirmParcelRequest {route.id(), 1, scenario.first, ordinal});
    }
    app.confirmStopParcel.execute(pum, routing::application::ConfirmParcelRequest {route.id(), 1, scenario.second, 1});

    app.resolveRouteStop.execute(
        pum,
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", false});

    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.second).status(), ShipmentStatus::Entregado);
}

TEST(ParcelsAndGroupsFlow, PartialDeliveryReschedulesTheGroupAndKeepsDeliveredParcelsAcrossARestart)
{
    test::Storage                                    storage;
    Scenario                                         scenario {storage};
    auto&                                            app   = scenario.app;
    const auto&                                      route = scenario.routeOfTheGroup();
    const auto                                       pum   = app.pum(scenario.courier);
    const routing::application::ConfirmParcelRequest one {route.id(), 1, scenario.first, 1};
    const routing::application::ConfirmParcelRequest two {route.id(), 1, scenario.first, 2};
    app.confirmStopParcel.execute(pum, one);
    const auto auditAfterOne = storage.audit.entries.size();
    app.confirmStopParcel.execute(pum, one);
    EXPECT_EQ(storage.audit.entries.size(), auditAfterOne);
    app.confirmStopParcel.execute(pum, two);

    app.resolveRouteStop.execute(
        pum,
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Rescheduled, "", false});

    const auto& first = app.getShipment.execute(app.operador(), scenario.first);
    EXPECT_EQ(first.status(), ShipmentStatus::Reprogramado);
    EXPECT_EQ(first.pendingParcelCount(), 1U);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.second).status(), ShipmentStatus::Reprogramado);
    EXPECT_NE(first.history().latest()->note.find("entrega parcial: 2 de 3"), std::string::npos);
    EXPECT_TRUE(app.listPendingShipments.execute(app.operador()).contains(&first));
    EXPECT_NE(app.findShipmentGroupOfShipment.execute(app.operador(), scenario.first), nullptr);

    test::ApplicationHarness restarted {storage};
    const auto&              restored = restarted.getShipment.execute(restarted.operador(), scenario.first);
    EXPECT_EQ(restored.pendingParcelCount(), 1U);
    EXPECT_TRUE(restored.parcel(0).delivered());
    EXPECT_FALSE(restored.parcel(2).delivered());
    ASSERT_NE(restarted.findShipmentGroupOfShipment.execute(restarted.operador(), scenario.second), nullptr);
    EXPECT_EQ(restarted.listShipmentGroups.execute(restarted.operador()).size(), 1U);
    bool parcelAudited {false};
    for (const auto& entry : storage.audit.entries)
    {
        parcelAudited = parcelAudited || entry.type == "shipment.parcel_delivered";
    }
    EXPECT_TRUE(parcelAudited);
}

TEST(ParcelsAndGroupsFlow, ReschedulingOneMemberReschedulesTheWholeGroup)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app = scenario.app;
    scenario.routeOfTheGroup();

    app.rescheduleShipment.execute(app.operador(), scenario.second, "Nadie en casa");

    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.first).status(), ShipmentStatus::Reprogramado);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.second).status(), ShipmentStatus::Reprogramado);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.first).attempts(), 1U);
}

TEST(ParcelsAndGroupsFlow, DispatchingTheHeadOfTheListDispatchesItsGroup)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app = scenario.app;
    app.formShipmentGroup.execute(app.operador(), listOf(scenario.first, scenario.second));

    app.dispatchNextShipment.execute(app.operador(), "");
    app.dispatchNextShipment.execute(app.operador(), "");

    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.first).status(), ShipmentStatus::EnReparto);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.second).status(), ShipmentStatus::EnReparto);
    EXPECT_EQ(app.getShipment.execute(app.operador(), scenario.alone).status(), ShipmentStatus::EnReparto);
    EXPECT_EQ(app.listPendingShipments.execute(app.operador()).size(), 0U);
}

TEST(ParcelsAndGroupsFlow, DissolvingAGroupIsRepeatableAndFreesItsMembers)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app   = scenario.app;
    const auto&   group = app.formShipmentGroup.execute(app.operador(), listOf(scenario.first, scenario.second));
    const auto    id    = group.id();
    EXPECT_EQ(storage.shipmentGroups.groups.size(), 1U);

    app.dissolveShipmentGroup.execute(app.operador(), id);

    EXPECT_EQ(storage.shipmentGroups.groups.size(), 0U);
    EXPECT_EQ(app.findShipmentGroupOfShipment.execute(app.operador(), scenario.first), nullptr);
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.dissolveShipmentGroup.execute(app.operador(), id);
                  }),
              ErrorKind::NotFound);
    const auto batchId = scenario.openBatch();
    EXPECT_NO_THROW(app.processSortingBatchItem.execute(app.operador(), batchId, listOf(scenario.first)));
}

TEST(ParcelsAndGroupsFlow, AGroupInDeliveryCannotBeDissolvedAndOnlyPlantStaffManagesGroups)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app = scenario.app;
    scenario.routeOfTheGroup();
    std::string groupId;
    app.listShipmentGroups.execute(app.operador())
        .forEach(
            [&groupId](const logistics::domain::ShipmentGroup& group)
            {
                groupId = group.id();
            });

    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.dissolveShipmentGroup.execute(app.operador(), groupId);
                  }),
              ErrorKind::Conflict);
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.formShipmentGroup.execute(app.pum(scenario.courier), listOf(scenario.alone, scenario.second));
                  }),
              ErrorKind::Forbidden);
}

TEST(ParcelsAndGroupsFlow, AnotherCourierCannotConfirmParcelsOfThisRoute)
{
    test::Storage storage;
    Scenario      scenario {storage};
    auto&         app   = scenario.app;
    const auto&   route = scenario.routeOfTheGroup();
    const auto    other = createCourier(app, "pum2@hubflow.local");

    const auto kind = kindOf(
        [&]
        {
            app.confirmStopParcel.execute(
                app.pum(other), routing::application::ConfirmParcelRequest {route.id(), 1, scenario.first, 1});
        });

    EXPECT_EQ(kind, ErrorKind::Forbidden);
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.confirmStopParcel.execute(
                          app.pum(scenario.courier),
                          routing::application::ConfirmParcelRequest {route.id(), 1, scenario.alone, 1});
                  }),
              ErrorKind::NotFound);
}
