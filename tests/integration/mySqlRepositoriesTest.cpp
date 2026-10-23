#include "hubflow/audit/adapters/out/mysql/mySqlAuditLog.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlUserRepository.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentRepository.hpp"
#include "hubflow/platform/config/settings.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRoadMapRepository.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRouteRepository.hpp"
#include "hubflow/shared/adapters/out/mysql/mySqlIdempotencyStore.hpp"

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
constexpr const char* MIGRATIONS[] {"001_schema.sql", "002_road_map.sql", "003_idempotency.sql"};
constexpr const char* TABLES[] {"idempotency_keys",
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
    std::ifstream     input {std::string {HUBFLOW_MIGRATIONS_DIR} + "/" + file};
    std::stringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

class ShipmentCollector final : public logistics::application::ShipmentRestorer
{
public:
    std::vector<logistics::domain::ShipmentState>                    states;
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
        settings.host           = valueOf("DB_HOST", settings.host);
        settings.port           = static_cast<std::uint16_t>(std::stoul(valueOf("DB_PORT", "3306")));
        settings.user           = valueOf("DB_USER", settings.user);
        settings.password       = valueOf("DB_PASSWORD", settings.password);
        settings.schema         = valueOf("DB_TEST_NAME", "hubflow_test");
        settings.connectRetries = 3;
        m_spConnection          = std::make_unique<platform::MySqlConnection>(settings);
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
        user.id           = users.nextIdentity();
        user.email        = "pum1@hubflow.local";
        user.name         = "Pum Uno";
        user.role         = identity::domain::Role::Pum;
        user.passwordHash = "hash";
        users.add(user);
        return user;
    }

    logistics::domain::ShipmentState addShipment(logistics::application::ShipmentRepository& shipments,
                                                 const std::string&                          code)
    {
        logistics::domain::ShipmentState state;
        state.draft = {
            code, "Marta Lopez", logistics::domain::Zone::Norte, 2.5, logistics::domain::ServiceLevel::Express, "N2"};
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
    identity::application::UserRepository&  users = usersAdapter;
    const auto                              first = addCourier(users);

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
    logistics::application::ShipmentRepository&  shipments = shipmentsAdapter;
    auto                                         state     = addShipment(shipments, "PKG-9001");
    shipments.appendMovement("PKG-9001",
                             logistics::domain::Movement {
                                 1, logistics::domain::ShipmentStatus::Recibido, "Recibido", "Operador", 1700000000});
    state.status   = logistics::domain::ShipmentStatus::Clasificado;
    state.attempts = 1;
    shipments.update(state);

    ShipmentCollector collector;
    shipments.loadAll(collector);

    ASSERT_EQ(collector.states.size(), 1U);
    EXPECT_EQ(collector.states[0].draft.code, "PKG-9001");
    EXPECT_EQ(collector.states[0].status, logistics::domain::ShipmentStatus::Clasificado);
    EXPECT_EQ(collector.states[0].attempts, 1U);
    EXPECT_EQ(collector.states[0].ticket, 7U);
    ASSERT_EQ(collector.movements.size(), 1U);
    EXPECT_EQ(collector.movements[0].second.note, "Recibido");
}

TEST_F(MySqlRepositories, RoadMapLoadsReferenceCity)
{
    routing::adapters::MySqlRoadMapRepository mapsAdapter {connection()};
    routing::application::RoadMapRepository&  maps = mapsAdapter;
    routing::domain::RoadMap                  map;

    maps.loadInto(map);

    EXPECT_EQ(map.nodeCount(), 15U);
    EXPECT_EQ(map.node(map.hub()).id, "HUB");
}

TEST_F(MySqlRepositories, RoutesAndStopOutcomesSurviveReload)
{
    identity::adapters::MySqlUserRepository      usersAdapter {connection()};
    identity::application::UserRepository&       users = usersAdapter;
    logistics::adapters::MySqlShipmentRepository shipmentsAdapter {connection()};
    logistics::application::ShipmentRepository&  shipments = shipmentsAdapter;
    routing::adapters::MySqlRouteRepository      routesAdapter {connection()};
    routing::application::RouteRepository&       routes  = routesAdapter;
    const auto                                   courier = addCourier(users);
    addShipment(shipments, "PKG-9001");
    routing::domain::RouteRecord record;
    record.id        = 1;
    record.courier   = {courier.id, courier.name};
    record.algorithm = routing::domain::SearchAlgorithm::AStar;
    record.createdAt = 1700000000;
    record.stops.pushBack(routing::domain::RouteStop {1, "PKG-9001", "N2", routing::domain::StopOutcome::Pending});
    routes.add(record);
    routes.updateStop(1, routing::domain::RouteStop {1, "PKG-9001", "N2", routing::domain::StopOutcome::Delivered});

    RouteCollector collector;
    routes.loadAll(collector);

    ASSERT_EQ(collector.records.size(), 1U);
    EXPECT_EQ(collector.records[0].courier.name, "Pum Uno");
    ASSERT_EQ(collector.records[0].stops.size(), 1U);
    EXPECT_EQ(collector.records[0].stops[0].outcome, routing::domain::StopOutcome::Delivered);
}

TEST_F(MySqlRepositories, AuditEntriesAreListedNewestFirst)
{
    audit::adapters::MySqlAuditLog logAdapter {connection()};
    audit::application::AuditLog&  log = logAdapter;
    log.append(audit::application::AuditEntry {0, 100, 1, "Admin", "USER_REGISTERED", "u1", "primero"});
    log.append(audit::application::AuditEntry {0, 200, 1, "Admin", "USER_REGISTERED", "u2", "segundo"});

    AuditCollector collector;
    log.forEachRecent(10, 0, collector);

    EXPECT_EQ(log.count(), 2U);
    ASSERT_EQ(collector.entries.size(), 2U);
    EXPECT_EQ(collector.entries[0].detail, "segundo");
}

TEST_F(MySqlRepositories, IdempotencyStoreKeepsTheFirstResponse)
{
    shared::adapters::MySqlIdempotencyStore storeAdapter {connection()};
    shared::application::IdempotencyStore&  store = storeAdapter;
    store.save(1, "clave", shared::application::StoredResponse {"POST", "/api/v1/dispatches", 201, "{\"a\":1}"});
    store.save(1, "clave", shared::application::StoredResponse {"POST", "/api/v1/dispatches", 201, "{\"a\":2}"});

    const auto found = store.find(1, "clave");

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->body, "{\"a\":1}");
    EXPECT_EQ(found->status, 201);
    EXPECT_FALSE(store.find(2, "clave").has_value());
}
