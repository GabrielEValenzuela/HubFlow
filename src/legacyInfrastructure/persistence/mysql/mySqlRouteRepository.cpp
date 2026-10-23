#include "infrastructure/persistence/mysql/mySqlRouteRepository.hpp"

#include "application/shared/vocabulary.hpp"
#include "infrastructure/persistence/mysql/sqlConversions.hpp"

#include <map>

namespace hubflow::infrastructure
{
namespace
{
using application::Vocabulary;

constexpr const char* INSERT_ROUTE {R"SQL(
    INSERT INTO routes (id, courier_id, courier_name, algorithm, created_at) VALUES (?, ?, ?, ?, ?)
)SQL"};

constexpr const char* INSERT_STOP {R"SQL(
    INSERT INTO route_stops (route_id, stop_order, shipment_code, node_id, outcome) VALUES (?, ?, ?, ?, ?)
)SQL"};

constexpr const char* UPDATE_STOP {R"SQL(
    UPDATE route_stops SET outcome = ? WHERE route_id = ? AND stop_order = ?
)SQL"};

constexpr const char* SELECT_ROUTES {R"SQL(
    SELECT id, courier_id, courier_name, algorithm, created_at FROM routes ORDER BY id
)SQL"};

constexpr const char* SELECT_STOPS {R"SQL(
    SELECT route_id, stop_order, shipment_code, node_id, outcome FROM route_stops ORDER BY route_id, stop_order
)SQL"};

constexpr const char* SELECT_NODES {R"SQL(
    SELECT id, label, is_hub, zone, x, y FROM road_nodes ORDER BY position
)SQL"};

constexpr const char* SELECT_EDGES {R"SQL(
    SELECT from_id, to_id, cost_km FROM road_edges ORDER BY id
)SQL"};
} // namespace

MySqlRouteRepository::MySqlRouteRepository(MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlRouteRepository::add(const domain::routing::RouteRecord& record)
{
    m_connection.execute(INSERT_ROUTE,
                         {record.id,
                          record.courier.id,
                          record.courier.name,
                          Vocabulary::name(record.algorithm),
                          std::int64_t {record.createdAt}});
    for (std::size_t i = 0; i < record.stops.size(); ++i)
    {
        const auto& stop = record.stops[i];
        m_connection.execute(INSERT_STOP,
                             {record.id,
                              std::uint64_t {stop.order},
                              stop.shipmentCode,
                              stop.nodeId,
                              Vocabulary::name(stop.outcome)});
    }
}

void MySqlRouteRepository::updateStop(const std::uint64_t routeId, const domain::routing::RouteStop& stop)
{
    m_connection.execute(UPDATE_STOP, {Vocabulary::name(stop.outcome), routeId, std::uint64_t {stop.order}});
}

void MySqlRouteRepository::loadAll(domain::routing::RouteRestorer& restorer) const
{
    std::map<std::uint64_t, domain::routing::RouteRecord> records;
    for (const auto& row : m_connection.query(SELECT_ROUTES))
    {
        domain::routing::RouteRecord record;
        record.id           = SqlColumns::unsignedNumber(row, 0);
        record.courier.id   = SqlColumns::unsignedNumber(row, 1);
        record.courier.name = SqlColumns::text(row, 2);
        record.algorithm    = SqlColumns::term(Vocabulary::searchAlgorithm(SqlColumns::text(row, 3)), "algorithm");
        record.createdAt    = SqlColumns::signedNumber(row, 4);
        records.emplace(record.id, record);
    }
    for (const auto& row : m_connection.query(SELECT_STOPS))
    {
        records.at(SqlColumns::unsignedNumber(row, 0))
            .stops.pushBack(domain::routing::RouteStop {
                static_cast<std::size_t>(SqlColumns::unsignedNumber(row, 1)),
                SqlColumns::text(row, 2),
                SqlColumns::text(row, 3),
                SqlColumns::term(Vocabulary::stopOutcome(SqlColumns::text(row, 4)), "outcome")});
    }
    for (const auto& [id, record] : records)
    {
        restorer.restoreRoute(record);
    }
}

MySqlRoadMapRepository::MySqlRoadMapRepository(MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlRoadMapRepository::loadInto(domain::routing::RoadMap& map) const
{
    for (const auto& row : m_connection.query(SELECT_NODES))
    {
        map.addNode(domain::routing::RoadNode {SqlColumns::text(row, 0),
                                               SqlColumns::text(row, 1),
                                               SqlColumns::flag(row, 2),
                                               SqlColumns::term(Vocabulary::zone(SqlColumns::text(row, 3)), "zone"),
                                               SqlColumns::decimal(row, 4),
                                               SqlColumns::decimal(row, 5)});
    }
    for (const auto& row : m_connection.query(SELECT_EDGES))
    {
        map.connect(SqlColumns::text(row, 0), SqlColumns::text(row, 1), SqlColumns::decimal(row, 2));
    }
}

} // namespace hubflow::infrastructure
