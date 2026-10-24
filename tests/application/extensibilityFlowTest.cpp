#include "support/applicationHarness.hpp"
#include "support/fixtures.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using shared::domain::BusinessError;
using shared::domain::ErrorKind;

namespace
{
    std::string codeOfFailure(const std::function<void()>& action)
    {
        std::string code;
        try
        {
            action();
        }
        catch (const BusinessError& error)
        {
            code = error.code();
        }
        return code;
    }

    std::uint64_t createCourier(test::ApplicationHarness& app, const std::string& email)
    {
        return app.registerUser
            .execute(app.admin(),
                     identity::application::RegisterUserRequest {
                         email, "Repartidor", identity::domain::Role::Pum, "clave-segura"})
            .id;
    }

    void openWestZone(test::ApplicationHarness& app)
    {
        app.createZone.execute(app.admin(), logistics::application::CreateZoneRequest {"OESTE", "Oeste"});
        routing::application::AddRoadNodeRequest plaza {"O1", "Plaza Oeste", "OESTE", 6.0, 10.0, {}};
        plaza.links.pushBack({"HUB", 4.5});
        app.addRoadNode.execute(app.admin(), plaza);
        routing::application::AddRoadNodeRequest parque {"O2", "Parque Oeste", "OESTE", 3.0, 12.0, {}};
        parque.links.pushBack({"O1", 4.0});
        app.addRoadNode.execute(app.admin(), parque);
    }

    logistics::domain::ShipmentDraft westShipment(const std::string& code, const std::string& service = "EXPRESS")
    {
        auto shipment = test::draft(code, service, "OESTE", 2.0);
        shipment.deliveryPoint = "O1";
        return shipment;
    }

    std::vector<std::string> pendingCodes(test::ApplicationHarness& app)
    {
        std::vector<std::string> codes;
        app.listPendingShipments.execute(app.operador())
            .forEach([&codes](const logistics::domain::Shipment& shipment) { codes.push_back(shipment.code()); });
        return codes;
    }

    bool audited(const test::Storage& storage, const std::string& type)
    {
        return std::any_of(storage.audit.entries.begin(),
                           storage.audit.entries.end(),
                           [&type](const audit::application::AuditEntry& entry) { return entry.type == type; });
    }

    const routing::domain::DeliveryRoute& assignNorthRoute(test::ApplicationHarness& app, const std::uint64_t courierId)
    {
        const auto& batch = app.openSortingBatch.execute(app.operador(), "NORTE");
        app.prepareSortingBatch.execute(app.operador(), batch.id());
        app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
        return app.assignDeliveryRoute.execute(
            app.operador(),
            routing::application::AssignRouteRequest {batch.id(), courierId, routing::domain::SearchAlgorithm::AStar});
    }
} // namespace

TEST(Extensibility, ANewZoneReceivesRoutesAndDeliversWithoutCodeChanges)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    openWestZone(app);
    const auto courierId = createCourier(app, "pum2@hubflow.local");

    app.registerShipment.execute(app.operador(), westShipment("PKG-3001"));

    const auto metrics = app.getOperationsMetrics.execute(app.operador());
    ASSERT_EQ(metrics.pendingByZone.size(), 4U);
    EXPECT_EQ(metrics.pendingByZone[3].code, "OESTE");
    EXPECT_EQ(metrics.pendingByZone[3].summary.count, 1U);
    EXPECT_EQ(app.summarizeZone.execute(app.operador(), "OESTE").count, 1U);

    const auto& batch = app.openSortingBatch.execute(app.operador(), "OESTE");
    app.prepareSortingBatch.execute(app.operador(), batch.id());
    app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
    const auto& route = app.assignDeliveryRoute.execute(
        app.operador(), routing::application::AssignRouteRequest {batch.id(), courierId, {}});
    EXPECT_EQ(route.stops()[0].nodeId, "O1");

    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-3001").status(),
              logistics::domain::ShipmentStatus::Entregado);

    EXPECT_EQ(storage.catalogs.zones.back().code, "OESTE");
    ASSERT_EQ(storage.maps.added.size(), 2U);
    EXPECT_EQ(storage.maps.added[0].node.id, "O1");
    EXPECT_TRUE(audited(storage, "zone.created"));
    EXPECT_TRUE(audited(storage, "road_node.added"));
}

TEST(Extensibility, ANewZoneAndItsMapSurviveARestart)
{
    test::Storage storage;
    {
        test::ApplicationHarness app {storage};
        openWestZone(app);
        app.registerShipment.execute(app.operador(), westShipment("PKG-3001"));
    }
    test::ApplicationHarness restored {storage};

    EXPECT_EQ(restored.listZones.execute(restored.admin()).size(), 4U);
    EXPECT_EQ(restored.getRoadMap.execute().node(restored.getRoadMap.execute().require("O2")).zone, "OESTE");
    EXPECT_EQ(pendingCodes(restored), (std::vector<std::string> {"PKG-3001"}));
    restored.registerShipment.execute(restored.operador(), westShipment("PKG-3002"));
    EXPECT_EQ(restored.summarizeZone.execute(restored.operador(), "OESTE").count, 2U);
}

TEST(Extensibility, OnlyAdministratorsConfigureZonesAndTheMap)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};

    EXPECT_EQ(
        codeOfFailure(
            [&]
            { app.createZone.execute(app.operador(), logistics::application::CreateZoneRequest {"OESTE", "Oeste"}); }),
        "auth.forbidden");
    EXPECT_EQ(
        codeOfFailure(
            [&] { app.updateRoadNode.execute(app.operador(), "N1", routing::application::UpdateRoadNodeRequest {}); }),
        "auth.forbidden");
    EXPECT_NO_THROW(app.listZones.execute(app.pum(7)));
}

TEST(Extensibility, DuplicateZoneAndInvalidMapPointsAreRejected)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    openWestZone(app);

    EXPECT_EQ(
        codeOfFailure(
            [&] { app.createZone.execute(app.admin(), logistics::application::CreateZoneRequest {"OESTE", "Otra"}); }),
        "zone.duplicate");
    routing::application::AddRoadNodeRequest unknownZone {"E1", "Este", "ESTE", 14.0, 10.0, {}};
    unknownZone.links.pushBack({"HUB", 4.5});
    EXPECT_EQ(codeOfFailure([&] { app.addRoadNode.execute(app.admin(), unknownZone); }), "road_node.zone_unavailable");
    routing::application::AddRoadNodeRequest repeated {"O1", "Repetido", "OESTE", 6.0, 11.0, {}};
    repeated.links.pushBack({"HUB", 5.0});
    EXPECT_THROW(app.addRoadNode.execute(app.admin(), repeated), BusinessError);
    EXPECT_EQ(app.getRoadMap.execute().nodeCount(), 17U);
}

TEST(Extensibility, DeactivatingAZoneFollowsTheDomainRules)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    openWestZone(app);
    const auto courierId = createCourier(app, "pum2@hubflow.local");
    const logistics::application::UpdateZoneRequest close {std::nullopt, false};
    app.registerShipment.execute(app.operador(), westShipment("PKG-3001"));

    EXPECT_EQ(codeOfFailure([&] { app.updateZone.execute(app.admin(), "OESTE", close); }), "zone.has_shipments");

    const auto& batch = app.openSortingBatch.execute(app.operador(), "OESTE");
    app.prepareSortingBatch.execute(app.operador(), batch.id());
    app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
    const auto& route = app.assignDeliveryRoute.execute(
        app.operador(), routing::application::AssignRouteRequest {batch.id(), courierId, {}});
    EXPECT_EQ(codeOfFailure([&] { app.updateZone.execute(app.admin(), "OESTE", close); }), "zone.has_routes");

    app.resolveRouteStop.execute(
        app.pum(courierId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});
    EXPECT_EQ(codeOfFailure([&] { app.updateZone.execute(app.admin(), "OESTE", close); }), "zone.has_map_nodes");

    app.updateRoadNode.execute(app.admin(), "O1", routing::application::UpdateRoadNodeRequest {std::nullopt, false});
    app.updateRoadNode.execute(
        app.admin(), "O2", routing::application::UpdateRoadNodeRequest {std::string {"CENTRO"}, std::nullopt});
    const auto& closed = app.updateZone.execute(app.admin(), "OESTE", close);
    EXPECT_FALSE(closed.isActive());
    EXPECT_FALSE(storage.catalogs.zones.back().active);
    EXPECT_TRUE(audited(storage, "zone.changed"));
    EXPECT_TRUE(storage.maps.changed.at("O2").zone == "CENTRO");

    EXPECT_EQ(codeOfFailure([&] { app.registerShipment.execute(app.operador(), westShipment("PKG-3002")); }),
              "shipment.zone_unavailable");
    EXPECT_EQ(codeOfFailure(
                  [&]
                  {
                      app.updateRoadNode.execute(
                          app.admin(), "O1", routing::application::UpdateRoadNodeRequest {std::nullopt, true});
                  }),
              "road_node.zone_unavailable");

    const auto auditedEvents = storage.audit.entries.size();
    app.updateZone.execute(app.admin(), "OESTE", close);
    EXPECT_EQ(storage.audit.entries.size(), auditedEvents);

    app.updateZone.execute(app.admin(), "OESTE", logistics::application::UpdateZoneRequest {std::nullopt, true});
    EXPECT_TRUE(storage.catalogs.zones.back().active);
}

TEST(Extensibility, ANewServiceWithHigherPriorityGoesFirstAndSurvivesARestart)
{
    test::Storage storage;
    {
        test::ApplicationHarness app {storage};
        app.seedDataset();
        app.createServiceLevel.execute(
            app.admin(), logistics::application::CreateServiceLevelRequest {"RELAMPAGO", "Relámpago", 10});
        app.registerShipment.execute(app.operador(), test::draft("PKG-3001", "RELAMPAGO", "NORTE", 1.0));

        EXPECT_EQ(pendingCodes(app).front(), "PKG-3001");
        EXPECT_EQ(app.dispatchNextShipment.execute(app.operador(), "").code(), "PKG-3001");
        EXPECT_EQ(pendingCodes(app).front(), "PKG-1002");
        EXPECT_EQ(app.summarizeZone.execute(app.operador(), "NORTE").urgentCount, 0U);
        EXPECT_TRUE(audited(storage, "service_level.created"));
    }
    test::ApplicationHarness restored {storage};
    restored.registerShipment.execute(restored.operador(), test::draft("PKG-3002", "RELAMPAGO", "NORTE", 1.0));
    EXPECT_EQ(pendingCodes(restored).front(), "PKG-3002");
    EXPECT_EQ(restored.summarizeZone.execute(restored.operador(), "NORTE").urgentCount, 1U);
}

TEST(Extensibility, TheSeedDatasetKeepsItsOriginalPendingOrder)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    EXPECT_EQ(pendingCodes(app),
              (std::vector<std::string> {
                  "PKG-1002", "PKG-1006", "PKG-1003", "PKG-1005", "PKG-1008", "PKG-1001", "PKG-1004", "PKG-1007"}));
}

TEST(Extensibility, AServiceInUseCannotBeDeactivatedAndAnInactiveOneIsNotOffered)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const logistics::application::UpdateServiceLevelRequest close {std::nullopt, false};

    EXPECT_EQ(codeOfFailure([&] { app.updateServiceLevel.execute(app.admin(), "EXPRESS", close); }),
              "service_level.has_shipments");
    app.createServiceLevel.execute(app.admin(),
                                   logistics::application::CreateServiceLevelRequest {"NOCTURNO", "Nocturno", 2});
    app.updateServiceLevel.execute(app.admin(), "NOCTURNO", close);

    EXPECT_EQ(codeOfFailure([&] { app.registerShipment.execute(app.operador(), test::draft("PKG-3001", "NOCTURNO")); }),
              "shipment.service_unavailable");
    EXPECT_FALSE(storage.catalogs.services.back().active);
}

TEST(Extensibility, FeaturesAttachToShipmentsAndArePersisted)
{
    test::Storage storage;
    {
        test::ApplicationHarness app {storage};
        app.createShipmentFeature.execute(
            app.admin(), logistics::application::CreateShipmentFeatureRequest {"DOCUMENTOS", "Documentos"});
        auto draft = test::draft("PKG-3001", "EXPRESS");
        draft.features.pushBack("FRAGIL");
        draft.features.pushBack("DOCUMENTOS");
        const auto& shipment = app.registerShipment.execute(app.operador(), draft);

        EXPECT_EQ(shipment.features().size(), 2U);
        EXPECT_EQ(storage.shipments.states.at("PKG-3001").draft.features.size(), 2U);

        app.updateShipmentFeature.execute(
            app.admin(), "FRAGIL", logistics::application::UpdateShipmentFeatureRequest {std::nullopt, false});
        auto another = test::draft("PKG-3002", "EXPRESS");
        another.features.pushBack("FRAGIL");
        EXPECT_EQ(codeOfFailure([&] { app.registerShipment.execute(app.operador(), another); }),
                  "shipment.feature_unavailable");
        another.features = {};
        another.features.pushBack("INEXISTENTE");
        EXPECT_EQ(codeOfFailure([&] { app.registerShipment.execute(app.operador(), another); }),
                  "shipment.feature_unavailable");
        EXPECT_TRUE(app.getShipment.execute(app.operador(), "PKG-3001").hasFeature("FRAGIL"));
        EXPECT_TRUE(audited(storage, "feature.created"));
    }
    test::ApplicationHarness restored {storage};
    const auto& shipment = restored.getShipment.execute(restored.operador(), "PKG-3001");
    EXPECT_TRUE(shipment.hasFeature("FRAGIL"));
    EXPECT_TRUE(shipment.hasFeature("DOCUMENTOS"));
    EXPECT_FALSE(restored.listShipmentFeatures.execute(restored.admin()).require("FRAGIL").isActive());
}

TEST(Extensibility, RolePermissionsAreLoadedFromData)
{
    test::Storage storage;
    storage.rolePermissions.grants.clear();
    storage.rolePermissions.grants.push_back(
        {identity::domain::Role::Admin, identity::domain::Permission::ManageCatalogs});
    storage.rolePermissions.grants.push_back(
        {identity::domain::Role::Operador, identity::domain::Permission::OperateShipments});
    storage.rolePermissions.grants.push_back(
        {identity::domain::Role::Operador, identity::domain::Permission::ManageCatalogs});
    test::ApplicationHarness app {storage};

    EXPECT_NO_THROW(app.createZone.execute(app.admin(), logistics::application::CreateZoneRequest {"OESTE", "Oeste"}));
    EXPECT_NO_THROW(app.createZone.execute(app.operador(), logistics::application::CreateZoneRequest {"ESTE", "Este"}));
    EXPECT_NO_THROW(app.registerShipment.execute(app.operador(), test::draft("PKG-3001", "EXPRESS")));
    EXPECT_EQ(codeOfFailure([&] { app.registerShipment.execute(app.admin(), test::draft("PKG-3002", "EXPRESS")); }),
              "auth.forbidden");
    EXPECT_EQ(codeOfFailure([&] { app.listPendingShipments.execute(app.pum(9)); }), "auth.forbidden");
}

TEST(Extensibility, ACourierWithARouteInProgressCannotBeDeactivatedUntilTheRouteIsReassigned)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    app.bootstrapUser(identity::application::RegisterUserRequest {
        "admin@hubflow.local", "Admin", identity::domain::Role::Admin, "clave-segura"});
    const auto firstId = createCourier(app, "pum1@hubflow.local");
    const auto secondId = createCourier(app, "pum2@hubflow.local");
    const auto routeId = assignNorthRoute(app, firstId).id();
    identity::application::UpdateUserRequest deactivate;
    deactivate.active = false;

    EXPECT_EQ(codeOfFailure([&] { app.updateUser.execute(app.admin(), firstId, deactivate); }),
              "user.has_route_in_progress");
    identity::application::UpdateUserRequest demote;
    demote.role = identity::domain::Role::Operador;
    EXPECT_EQ(codeOfFailure([&] { app.updateUser.execute(app.admin(), firstId, demote); }),
              "user.has_route_in_progress");
    EXPECT_TRUE(storage.users.users.at(firstId).active);

    const auto events = storage.audit.entries.size();
    const auto& moved = app.reassignDeliveryRoute.execute(app.operador(), routeId, secondId);
    EXPECT_EQ(moved.courier().id, secondId);
    EXPECT_EQ(storage.routes.records.at(routeId).courier.id, secondId);
    EXPECT_EQ(storage.audit.entries.size(), events + 1);
    EXPECT_EQ(storage.audit.entries.back().type, "route.reassigned");

    app.reassignDeliveryRoute.execute(app.operador(), routeId, secondId);
    EXPECT_EQ(storage.audit.entries.size(), events + 1);

    EXPECT_FALSE(app.updateUser.execute(app.admin(), firstId, deactivate).active);
    EXPECT_EQ(app.listRoutes.execute(app.pum(firstId)).size(), 0U);
    EXPECT_EQ(app.listRoutes.execute(app.pum(secondId)).size(), 1U);
    EXPECT_EQ(codeOfFailure([&] { app.reassignDeliveryRoute.execute(app.operador(), routeId, firstId); }),
              "route.courier_invalid");

    test::ApplicationHarness restored {storage};
    EXPECT_EQ(restored.getRoute.execute(restored.admin(), routeId).courier().id, secondId);
}

TEST(Extensibility, OnlyPlantStaffReassignsAndFinishedRoutesStay)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    app.bootstrapUser(identity::application::RegisterUserRequest {
        "admin@hubflow.local", "Admin", identity::domain::Role::Admin, "clave-segura"});
    const auto firstId = createCourier(app, "pum1@hubflow.local");
    const auto secondId = createCourier(app, "pum2@hubflow.local");
    const auto& route = assignNorthRoute(app, firstId);

    EXPECT_EQ(codeOfFailure([&] { app.reassignDeliveryRoute.execute(app.pum(firstId), route.id(), secondId); }),
              "auth.forbidden");
    EXPECT_EQ(codeOfFailure([&] { app.reassignDeliveryRoute.execute(app.admin(), 999, secondId); }), "route.not_found");
    app.resolveRouteStop.execute(
        app.pum(firstId),
        routing::application::ResolveStopRequest {route.id(), 1, routing::domain::StopOutcome::Delivered, "", true});
    EXPECT_EQ(codeOfFailure([&] { app.reassignDeliveryRoute.execute(app.admin(), route.id(), secondId); }),
              "route.completed");
    identity::application::UpdateUserRequest deactivate;
    deactivate.active = false;
    EXPECT_FALSE(app.updateUser.execute(app.admin(), firstId, deactivate).active);
}

TEST(IdempotencyContract, ConfigurationUseCasesDeclareTheirIdempotency)
{
    using shared::application::Idempotency;
    EXPECT_EQ(logistics::application::CreateZoneUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::UpdateZoneUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(logistics::application::CreateServiceLevelUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::UpdateServiceLevelUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(logistics::application::CreateShipmentFeatureUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::UpdateShipmentFeatureUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(logistics::application::ListZonesUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(routing::application::AddRoadNodeUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(routing::application::UpdateRoadNodeUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(routing::application::ReassignDeliveryRouteUseCase::IDEMPOTENCY, Idempotency::Idempotent);
}
