#include "hubflow/audit/adapters/out/mysql/mySqlAuditLog.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlRolePermissionRepository.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlUserRepository.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlCatalogRepository.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentGroupRepository.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentRepository.hpp"
#include "hubflow/platform/config/settings.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRoadMapRepository.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRouteRepository.hpp"
#include "hubflow/shared/adapters/out/mysql/mySqlIdempotencyStore.hpp"
#include "hubflow/tracking/adapters/out/mysql/mySqlCustomerNoticeLog.hpp"
#include "hubflow/tracking/adapters/out/security/hmacTrackingTokenIssuer.hpp"

#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace hubflow;

namespace
{
    constexpr const char* MIGRATIONS[] {"001_schema.sql",
                                        "002_road_map.sql",
                                        "003_idempotency.sql",
                                        "004_catalogs.sql",
                                        "005_shipments_parcels_groups.sql",
                                        "006_customer_tracking.sql"};
    constexpr const char* TABLES[] {"notifications",
                                    "route_stop_companions",
                                    "shipment_group_members",
                                    "shipment_groups",
                                    "parcels",
                                    "shipment_feature_links",
                                    "shipment_features",
                                    "service_levels",
                                    "zones",
                                    "role_permissions",
                                    "idempotency_keys",
                                    "audit_events",
                                    "route_stops",
                                    "routes",
                                    "movements",
                                    "shipments",
                                    "road_edges",
                                    "road_nodes",
                                    "users"};

    std::string valueOf(const char* pName, const std::string& fallback)
    {
        const char* pValue = std::getenv(pName);
        return pValue == nullptr || *pValue == '\0' ? fallback : std::string {pValue};
    }

    std::string contentsOf(const std::string& file)
    {
        std::ifstream input {std::string {HUBFLOW_MIGRATIONS_DIR} + "/" + file};
        std::stringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    class ShipmentCollector final : public logistics::application::ShipmentRestorer
    {
    public:
        std::vector<logistics::domain::ShipmentState> states;
        std::vector<std::pair<std::string, logistics::domain::Movement>> movements;

    private:
        void restoreShipment(const logistics::domain::ShipmentState& state) override
        {
            states.push_back(state);
        }

        void restoreMovement(const std::string& code, const logistics::domain::Movement& movement) override
        {
            movements.emplace_back(code, movement);
        }
    };

    class GroupCollector final : public logistics::application::ShipmentGroupRestorer
    {
    public:
        std::vector<logistics::domain::ShipmentGroupState> groups;

    private:
        void restoreGroup(const logistics::domain::ShipmentGroupState& state) override
        {
            groups.push_back(state);
        }
    };

    logistics::domain::ShipmentDraft draftOf(const std::string& code, const std::string& id)
    {
        logistics::domain::ShipmentDraft draft;
        draft.id = id;
        draft.code = code;
        draft.recipient = "Marta Lopez";
        draft.recipientPhone = "1155550000";
        draft.recipientPostalCode = "1640";
        draft.zone = "NORTE";
        draft.service = "EXPRESS";
        draft.deliveryPoint = "N2";
        draft.parcels.pushBack(logistics::domain::ParcelData {2.5, 30.0, 20.0, 10.0});
        return draft;
    }

    class CatalogCollector final : public logistics::application::CatalogRestorer
    {
    public:
        std::vector<logistics::domain::ZoneState> zones;
        std::vector<logistics::domain::ServiceLevelState> services;
        std::vector<logistics::domain::ShipmentFeatureState> features;

    private:
        void restoreZone(const logistics::domain::ZoneState& state) override
        {
            zones.push_back(state);
        }

        void restoreServiceLevel(const logistics::domain::ServiceLevelState& state) override
        {
            services.push_back(state);
        }

        void restoreFeature(const logistics::domain::ShipmentFeatureState& state) override
        {
            features.push_back(state);
        }
    };

    class GrantCollector final : public identity::application::GrantRestorer
    {
    public:
        std::vector<std::pair<identity::domain::Role, identity::domain::Permission>> grants;

    private:
        void restoreGrant(const identity::domain::Role role, const identity::domain::Permission permission) override
        {
            grants.emplace_back(role, permission);
        }
    };

    class RouteCollector final : public routing::application::RouteRestorer
    {
    public:
        std::vector<routing::domain::RouteRecord> records;

    private:
        void restoreRoute(const routing::domain::RouteRecord& record) override
        {
            records.push_back(record);
        }
    };

    class AuditCollector final : public audit::application::AuditEntryVisitor
    {
    public:
        std::vector<audit::application::AuditEntry> entries;

    private:
        void visit(const audit::application::AuditEntry& entry) override
        {
            entries.push_back(entry);
        }
    };

    class MySqlRepositories : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            if (std::getenv("DB_HOST") == nullptr)
            {
                GTEST_SKIP() << "DB_HOST no está definido: se omiten las pruebas con base de datos";
            }
            platform::DatabaseSettings settings;
            settings.host = valueOf("DB_HOST", settings.host);
            settings.port = static_cast<std::uint16_t>(std::stoul(valueOf("DB_PORT", "3306")));
            settings.user = valueOf("DB_USER", settings.user);
            settings.password = valueOf("DB_PASSWORD", settings.password);
            settings.schema = valueOf("DB_TEST_NAME", "hubflow_test");
            settings.connectRetries = 3;
            m_spConnection = std::make_unique<platform::MySqlConnection>(settings);
            m_spConnection->executeScript("SET FOREIGN_KEY_CHECKS = 0");
            for (const char* pTable : TABLES)
            {
                m_spConnection->executeScript(std::string {"DROP TABLE IF EXISTS "} + pTable);
            }
            m_spConnection->executeScript("SET FOREIGN_KEY_CHECKS = 1");
            for (const char* pFile : MIGRATIONS)
            {
                m_spConnection->executeScript(contentsOf(pFile));
            }
        }

        platform::MySqlConnection& connection()
        {
            return *m_spConnection;
        }

        identity::domain::UserState addCourier(identity::application::UserRepository& users)
        {
            identity::domain::UserState user;
            user.id = users.nextIdentity();
            user.email = "pum1@hubflow.local";
            user.name = "Pum Uno";
            user.role = identity::domain::Role::Pum;
            user.passwordHash = "hash";
            users.add(user);
            return user;
        }

        logistics::domain::ShipmentState addShipment(logistics::application::ShipmentRepository& shipments,
                                                     const std::string& code)
        {
            logistics::domain::ShipmentState state;
            state.draft =
                draftOf(code,
                        "018f0000-0000-7000-8000-" + std::string(12 - std::min<std::size_t>(12, code.size() - 4), '0') +
                            code.substr(4));
            state.ticket = 7;
            shipments.add(state);
            return state;
        }

    private:
        std::unique_ptr<platform::MySqlConnection> m_spConnection;
    };
} // namespace

TEST_F(MySqlRepositories, UsersRoundTripByIdAndEmail)
{
    identity::adapters::MySqlUserRepository usersAdapter {connection()};
    identity::application::UserRepository& users = usersAdapter;
    const auto first = addCourier(users);

    auto stored = users.byEmail("pum1@hubflow.local");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->id, first.id);
    EXPECT_EQ(stored->role, identity::domain::Role::Pum);
    stored->active = false;
    users.update(*stored);
    EXPECT_FALSE(users.byId(first.id)->active);
    EXPECT_EQ(users.count(), 1U);
    EXPECT_GT(users.nextIdentity(), first.id);
    EXPECT_FALSE(users.byEmail("nadie@hubflow.local").has_value());
}

TEST_F(MySqlRepositories, ShipmentsAndMovementsSurviveReload)
{
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository& shipments = shipmentsAdapter;
    auto state = addShipment(shipments, "PKG-9001");
    shipments.appendMovement("PKG-9001",
                             logistics::domain::Movement {
                                 1, logistics::domain::ShipmentStatus::Recibido, "Recibido", "Operador", 1700000000});
    state.status = logistics::domain::ShipmentStatus::Clasificado;
    state.attempts = 1;
    shipments.update(state);

    ShipmentCollector collector;
    shipments.loadAll(collector);

    ASSERT_EQ(collector.states.size(), 1U);
    EXPECT_EQ(collector.states[0].draft.code, "PKG-9001");
    EXPECT_EQ(collector.states[0].draft.id, "018f0000-0000-7000-8000-000000009001");
    EXPECT_EQ(collector.states[0].draft.recipientPhone, "1155550000");
    EXPECT_EQ(collector.states[0].draft.recipientPostalCode, "1640");
    EXPECT_EQ(collector.states[0].status, logistics::domain::ShipmentStatus::Clasificado);
    EXPECT_EQ(collector.states[0].attempts, 1U);
    EXPECT_EQ(collector.states[0].ticket, 7U);
    ASSERT_EQ(collector.movements.size(), 1U);
    EXPECT_EQ(collector.movements[0].second.note, "Recibido");
}

TEST_F(MySqlRepositories, ParcelsAndTheirDeliveryFlagsSurviveReload)
{
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository& shipments = shipmentsAdapter;
    logistics::domain::ShipmentState state;
    state.draft = draftOf("HF-7K3M9Q2X8", "018f0000-0000-7000-8000-00000000a001");
    state.draft.parcels.pushBack(logistics::domain::ParcelData {1.25, 60.0, 50.0, 40.0});
    state.ticket = 1;
    shipments.add(state);
    state.draft.parcels[1].delivered = true;
    shipments.update(state);
    ShipmentCollector collector;

    shipments.loadAll(collector);

    ASSERT_EQ(collector.states.size(), 1U);
    const auto& parcels = collector.states[0].draft.parcels;
    ASSERT_EQ(parcels.size(), 2U);
    EXPECT_DOUBLE_EQ(parcels[0].weightKg, 2.5);
    EXPECT_FALSE(parcels[0].delivered);
    EXPECT_DOUBLE_EQ(parcels[1].weightKg, 1.25);
    EXPECT_DOUBLE_EQ(parcels[1].heightCm, 40.0);
    EXPECT_TRUE(parcels[1].delivered);
}

TEST_F(MySqlRepositories, ShipmentGroupsSurviveReloadAndCanBeRemoved)
{
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository& shipments = shipmentsAdapter;
    logistics::adapters::MySqlShipmentGroupRepository groupsAdapter {connection()};
    logistics::application::ShipmentGroupRepository& groups = groupsAdapter;
    addShipment(shipments, "PKG-9001");
    addShipment(shipments, "PKG-9002");
    logistics::domain::ShipmentGroupState group;
    group.id = "018f0000-0000-7000-8000-00000000b001";
    group.deliveryPoint = "N2";
    group.codes.pushBack("PKG-9002");
    group.codes.pushBack("PKG-9001");
    groups.add(group);
    GroupCollector collector;

    groups.loadAll(collector);

    ASSERT_EQ(collector.groups.size(), 1U);
    EXPECT_EQ(collector.groups[0].id, group.id);
    EXPECT_EQ(collector.groups[0].codes[0], "PKG-9002");
    groups.remove(group.id);
    GroupCollector afterwards;
    groups.loadAll(afterwards);
    EXPECT_TRUE(afterwards.groups.empty());
}

TEST_F(MySqlRepositories, RoadMapLoadsReferenceCity)
{
    routing::adapters::MySqlRoadMapRepository mapsAdapter {connection()};
    routing::application::RoadMapRepository& maps = mapsAdapter;
    routing::domain::RoadMap map;

    maps.loadInto(map);

    EXPECT_EQ(map.nodeCount(), 15U);
    EXPECT_EQ(map.node(map.hub()).id, "HUB");
}

TEST_F(MySqlRepositories, RoutesAndStopOutcomesSurviveReload)
{
    identity::adapters::MySqlUserRepository usersAdapter {connection()};
    identity::application::UserRepository& users = usersAdapter;
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository& shipments = shipmentsAdapter;
    routing::adapters::MySqlRouteRepository routesAdapter {connection()};
    routing::application::RouteRepository& routes = routesAdapter;
    const auto courier = addCourier(users);
    addShipment(shipments, "PKG-9001");
    routing::domain::RouteRecord record;
    record.id = 1;
    record.courier = {courier.id, courier.name};
    record.algorithm = routing::domain::SearchAlgorithm::AStar;
    record.createdAt = 1700000000;
    addShipment(shipments, "PKG-9002");
    routing::domain::RouteStop stop {1, "PKG-9001", "N2", routing::domain::StopOutcome::Pending};
    stop.companions.pushBack("PKG-9002");
    record.stops.pushBack(stop);
    routes.add(record);
    stop.outcome = routing::domain::StopOutcome::Delivered;
    routes.updateStop(1, stop);

    RouteCollector collector;
    routes.loadAll(collector);

    ASSERT_EQ(collector.records.size(), 1U);
    EXPECT_EQ(collector.records[0].courier.name, "Pum Uno");
    ASSERT_EQ(collector.records[0].stops.size(), 1U);
    EXPECT_EQ(collector.records[0].stops[0].outcome, routing::domain::StopOutcome::Delivered);
    ASSERT_EQ(collector.records[0].stops[0].companions.size(), 1U);
    EXPECT_EQ(collector.records[0].stops[0].companions[0], "PKG-9002");
}

TEST_F(MySqlRepositories, AuditEntriesAreListedNewestFirst)
{
    audit::adapters::MySqlAuditLog logAdapter {connection()};
    audit::application::AuditLog& log = logAdapter;
    log.append(audit::application::AuditEntry {0, 100, 1, "Admin", "USER_REGISTERED", "u1", "primero"});
    log.append(audit::application::AuditEntry {0, 200, 1, "Admin", "USER_REGISTERED", "u2", "segundo"});

    AuditCollector collector;
    log.forEachRecent(10, 0, collector);

    EXPECT_EQ(log.count(), 2U);
    ASSERT_EQ(collector.entries.size(), 2U);
    EXPECT_EQ(collector.entries[0].detail, "segundo");
}

class NoticeCollector final : public tracking::application::CustomerNoticeVisitor
{
public:
    std::vector<tracking::application::StoredCustomerNotice> notices;

private:
    void visit(const tracking::application::StoredCustomerNotice& stored) override
    {
        notices.push_back(stored);
    }
};

TEST_F(MySqlRepositories, CustomerNoticesAreStoredMaskedAndListedNewestFirst)
{
    tracking::adapters::MySqlCustomerNoticeLog logAdapter {connection()};
    tracking::application::CustomerNoticeLog& log = logAdapter;
    log.append(
        tracking::application::CustomerNotice {"PKG-9001", "SMS", "****4410", "Paquete PKG-9001: Está en camino", 100});
    log.append(
        tracking::application::CustomerNotice {"PKG-9001", "SMS", "****4410", "Paquete PKG-9001: Entregado", 200});

    NoticeCollector collector;
    log.forEachRecent(1, 0, collector);

    EXPECT_EQ(log.count(), 2U);
    ASSERT_EQ(collector.notices.size(), 1U);
    EXPECT_EQ(collector.notices[0].notice.message, "Paquete PKG-9001: Entregado");
    EXPECT_EQ(collector.notices[0].notice.recipient, "****4410");
    EXPECT_EQ(collector.notices[0].notice.sentAt, 200);
}

TEST_F(MySqlRepositories, TheTrackingTokenIsSignedScopedAndExpires)
{
    const tracking::adapters::HmacTrackingTokenIssuer issuerAdapter {"un-secreto-de-prueba-de-al-menos-32-caracteres"};
    const tracking::application::TrackingTokenIssuer& issuer = issuerAdapter;
    const std::string token = issuer.issue(tracking::application::TrackingGrant {"PKG-9001", 2000});

    const auto valid = issuer.verify(token, 1000);
    EXPECT_TRUE(valid.has_value());
    EXPECT_EQ(valid->code, "PKG-9001");
    EXPECT_FALSE(issuer.verify(token, 2000).has_value());
    EXPECT_FALSE(issuer.verify(token + "x", 1000).has_value());
    EXPECT_FALSE(issuer.verify("garbage", 1000).has_value());
    std::string forged {token};
    forged[0] = forged[0] == 'A' ? 'B' : 'A';
    EXPECT_FALSE(issuer.verify(forged, 1000).has_value());
    const tracking::adapters::HmacTrackingTokenIssuer other {"otro-secreto-distinto-de-al-menos-32-caracteres"};
    EXPECT_FALSE(static_cast<const tracking::application::TrackingTokenIssuer&>(other).verify(token, 1000).has_value());
}

TEST_F(MySqlRepositories, IdempotencyStoreKeepsTheFirstResponse)
{
    shared::adapters::MySqlIdempotencyStore storeAdapter {connection()};
    shared::application::IdempotencyStore& store = storeAdapter;
    store.save(1, "clave", shared::application::StoredResponse {"POST", "/api/v1/dispatches", 201, "{\"a\":1}"});
    store.save(1, "clave", shared::application::StoredResponse {"POST", "/api/v1/dispatches", 201, "{\"a\":2}"});

    const auto found = store.find(1, "clave");

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->body, "{\"a\":1}");
    EXPECT_EQ(found->status, 201);
    EXPECT_FALSE(store.find(2, "clave").has_value());
}

TEST_F(MySqlRepositories, MigrationSeedsTheReferenceCatalogsAndPermissions)
{
    logistics::adapters::MySqlCatalogRepository catalogsAdapter {connection()};
    logistics::application::CatalogRepository& catalogs = catalogsAdapter;
    identity::adapters::MySqlRolePermissionRepository grantsAdapter {connection()};
    identity::application::RolePermissionRepository& grants = grantsAdapter;
    CatalogCollector collector;
    GrantCollector permissions;

    catalogs.loadAll(collector);
    grants.loadAll(permissions);

    ASSERT_EQ(collector.zones.size(), 3U);
    EXPECT_EQ(collector.zones[0].code, "CENTRO");
    ASSERT_EQ(collector.services.size(), 3U);
    EXPECT_EQ(collector.services[0].code, "EXPRESS");
    EXPECT_EQ(collector.services[0].priority, 3U);
    EXPECT_EQ(collector.features.size(), 3U);
    EXPECT_EQ(permissions.grants.size(), 14U);
}

TEST_F(MySqlRepositories, CatalogChangesAreUpsertedAndKeepTheirOrder)
{
    logistics::adapters::MySqlCatalogRepository catalogsAdapter {connection()};
    logistics::application::CatalogRepository& catalogs = catalogsAdapter;
    catalogs.saveZone({"OESTE", "Oeste", true});
    catalogs.saveZone({"NORTE", "Zona Norte", false});
    catalogs.saveServiceLevel({"RELAMPAGO", "Relámpago", 10, true});
    catalogs.saveFeature({"DOCUMENTOS", "Documentos", true});
    CatalogCollector collector;

    catalogs.loadAll(collector);

    ASSERT_EQ(collector.zones.size(), 4U);
    EXPECT_EQ(collector.zones[1].name, "Zona Norte");
    EXPECT_FALSE(collector.zones[1].active);
    EXPECT_EQ(collector.zones[3].code, "OESTE");
    EXPECT_EQ(collector.services.back().priority, 10U);
    EXPECT_EQ(collector.features.back().code, "DOCUMENTOS");
}

TEST_F(MySqlRepositories, ShipmentFeaturesSurviveReload)
{
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository& shipments = shipmentsAdapter;
    logistics::domain::ShipmentState state;
    state.draft = draftOf("PKG-9001", "018f0000-0000-7000-8000-000000009001");
    state.draft.features.pushBack("FRAGIL");
    state.draft.features.pushBack("REFRIGERADO");
    state.ticket = 1;
    shipments.add(state);
    ShipmentCollector collector;

    shipments.loadAll(collector);

    ASSERT_EQ(collector.states.size(), 1U);
    ASSERT_EQ(collector.states[0].draft.features.size(), 2U);
    EXPECT_EQ(collector.states[0].draft.features[0], "FRAGIL");
}

TEST_F(MySqlRepositories, NewMapPointsAndTheirChangesSurviveReload)
{
    logistics::adapters::MySqlCatalogRepository catalogsAdapter {connection()};
    logistics::application::CatalogRepository& catalogs = catalogsAdapter;
    catalogs.saveZone({"OESTE", "Oeste", true});
    routing::adapters::MySqlRoadMapRepository mapsAdapter {connection()};
    routing::application::RoadMapRepository& maps = mapsAdapter;
    ds::DynamicArray<routing::domain::RoadLink> links;
    links.pushBack({"HUB", 4.5});
    maps.addNode({"O1", "Plaza Oeste", false, "OESTE", 6.0, 10.0, true}, links);
    maps.updateNode({"O1", "Plaza Oeste", false, "CENTRO", 6.0, 10.0, false});
    routing::domain::RoadMap map;

    maps.loadInto(map);

    EXPECT_EQ(map.nodeCount(), 16U);
    const auto& added = map.node(map.require("O1"));
    EXPECT_EQ(added.zone, "CENTRO");
    EXPECT_FALSE(added.active);
    EXPECT_EQ(map.graph().degree(map.require("HUB")), 5U);
}

TEST_F(MySqlRepositories, ReassigningARouteIsPersisted)
{
    identity::adapters::MySqlUserRepository usersAdapter {connection()};
    identity::application::UserRepository& users = usersAdapter;
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository& shipments = shipmentsAdapter;
    routing::adapters::MySqlRouteRepository routesAdapter {connection()};
    routing::application::RouteRepository& routes = routesAdapter;
    const auto courier = addCourier(users);
    identity::domain::UserState other;
    other.id = users.nextIdentity();
    other.email = "pum2@hubflow.local";
    other.name = "Pum Dos";
    other.role = identity::domain::Role::Pum;
    other.passwordHash = "hash";
    users.add(other);
    addShipment(shipments, "PKG-9001");
    routing::domain::RouteRecord record;
    record.id = 1;
    record.courier = {courier.id, courier.name};
    record.createdAt = 1700000000;
    record.stops.pushBack(routing::domain::RouteStop {1, "PKG-9001", "N2", routing::domain::StopOutcome::Pending});
    routes.add(record);

    routes.updateCourier(1, {other.id, other.name});
    RouteCollector collector;
    routes.loadAll(collector);

    ASSERT_EQ(collector.records.size(), 1U);
    EXPECT_EQ(collector.records[0].courier.id, other.id);
    EXPECT_EQ(collector.records[0].courier.name, "Pum Dos");
}
