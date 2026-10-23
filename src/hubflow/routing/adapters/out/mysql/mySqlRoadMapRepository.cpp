#include "hubflow/routing/adapters/out/mysql/mySqlRoadMapRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"

namespace hubflow::routing::adapters
{
namespace
{
using platform::SqlColumns;

constexpr const char* SELECT_NODES {R"SQL(
    SELECT id, label, is_hub, zone, x, y, active FROM road_nodes ORDER BY position
)SQL"};

constexpr const char* NEXT_POSITION {"SELECT COALESCE(MAX(position), 0) + 1 FROM road_nodes"};

constexpr const char* INSERT_NODE {R"SQL(
    INSERT INTO road_nodes (id, position, label, is_hub, zone, x, y, active) VALUES (?, ?, ?, 0, ?, ?, ?, 1)
)SQL"};

constexpr const char* INSERT_EDGE {R"SQL(
    INSERT INTO road_edges (from_id, to_id, cost_km) VALUES (?, ?, ?)
)SQL"};

constexpr const char* UPDATE_NODE {R"SQL(
    UPDATE road_nodes SET zone = ?, active = ? WHERE id = ?
)SQL"};

constexpr const char* SELECT_EDGES {R"SQL(
    SELECT from_id, to_id, cost_km FROM road_edges ORDER BY id
)SQL"};
} // namespace

MySqlRoadMapRepository::MySqlRoadMapRepository(platform::MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlRoadMapRepository::loadInto(domain::RoadMap& map) const
{
    for (const auto& row : m_connection.query(SELECT_NODES))
    {
        map.addNode(domain::RoadNode {SqlColumns::text(row, 0),
                                      SqlColumns::text(row, 1),
                                      SqlColumns::flag(row, 2),
                                      SqlColumns::text(row, 3),
                                      SqlColumns::decimal(row, 4),
                                      SqlColumns::decimal(row, 5),
                                      SqlColumns::flag(row, 6)});
    }
    for (const auto& row : m_connection.query(SELECT_EDGES))
    {
        map.connect(SqlColumns::text(row, 0), SqlColumns::text(row, 1), SqlColumns::decimal(row, 2));
    }
}

void MySqlRoadMapRepository::addNode(const domain::RoadNode& node, const ds::DynamicArray<domain::RoadLink>& links)
{
    const std::uint64_t position {SqlColumns::unsignedNumber(m_connection.query(NEXT_POSITION).front(), 0)};
    m_connection.execute(INSERT_NODE, {node.id, position, node.label, node.zone, node.x, node.y});
    for (std::size_t i = 0; i < links.size(); ++i)
    {
        m_connection.execute(INSERT_EDGE, {node.id, links[i].toId, links[i].costKm});
    }
}

void MySqlRoadMapRepository::updateNode(const domain::RoadNode& node)
{
    m_connection.execute(UPDATE_NODE, {node.zone, std::int64_t {node.active ? 1 : 0}, node.id});
}

} // namespace hubflow::routing::adapters
