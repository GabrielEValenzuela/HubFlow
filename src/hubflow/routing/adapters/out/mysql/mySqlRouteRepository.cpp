#include "hubflow/routing/adapters/out/mysql/mySqlRouteRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

#include <map>

namespace hubflow::routing::adapters
{
namespace
{
using platform::SqlColumns;
using shared::application::Vocabulary;

constexpr const char* INSERT_ROUTE {R"SQL(
    INSERT INTO routes (id, courier_id, courier_name, algorithm, created_at) VALUES (?, ?, ?, ?, ?)
)SQL"};

constexpr const char* INSERT_STOP {R"SQL(
    INSERT INTO route_stops (route_id, stop_order, shipment_code, node_id, outcome) VALUES (?, ?, ?, ?, ?)
)SQL"};

constexpr const char* UPDATE_STOP {R"SQL(
    UPDATE route_stops SET outcome = ? WHERE route_id = ? AND stop_order = ?
)SQL"};

constexpr const char* UPDATE_COURIER {R"SQL(
    UPDATE routes SET courier_id = ?, courier_name = ? WHERE id = ?
)SQL"};

constexpr const char* SELECT_ROUTES {R"SQL(
    SELECT id, courier_id, courier_name, algorithm, created_at FROM routes ORDER BY id
)SQL"};

constexpr const char* SELECT_STOPS {R"SQL(
    SELECT route_id, stop_order, shipment_code, node_id, outcome FROM route_stops ORDER BY route_id, stop_order
)SQL"};
} // namespace

MySqlRouteRepository::MySqlRouteRepository(platform::MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlRouteRepository::add(const domain::RouteRecord& record)
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
        m_connection.execute(
            INSERT_STOP,
            {record.id, std::uint64_t {stop.order}, stop.shipmentCode, stop.nodeId, Vocabulary::name(stop.outcome)});
    }
}

void MySqlRouteRepository::updateStop(const std::uint64_t routeId, const domain::RouteStop& stop)
{
    m_connection.execute(UPDATE_STOP, {Vocabulary::name(stop.outcome), routeId, std::uint64_t {stop.order}});
}

void MySqlRouteRepository::updateCourier(const std::uint64_t routeId, const domain::Courier& courier)
{
    m_connection.execute(UPDATE_COURIER, {courier.id, courier.name, routeId});
}

void MySqlRouteRepository::loadAll(application::RouteRestorer& restorer) const
{
    std::map<std::uint64_t, domain::RouteRecord> records;
    for (const auto& row : m_connection.query(SELECT_ROUTES))
    {
        domain::RouteRecord record;
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
            .stops.pushBack(
                domain::RouteStop {static_cast<std::size_t>(SqlColumns::unsignedNumber(row, 1)),
                                   SqlColumns::text(row, 2),
                                   SqlColumns::text(row, 3),
                                   SqlColumns::term(Vocabulary::stopOutcome(SqlColumns::text(row, 4)), "outcome")});
    }
    for (const auto& [id, record] : records)
    {
        restorer.restoreRoute(record);
    }
}

} // namespace hubflow::routing::adapters
