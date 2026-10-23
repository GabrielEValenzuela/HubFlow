#include "hubflow/routing/adapters/out/mysql/mySqlRoadMapRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::routing::adapters
{
namespace
{
using platform::SqlColumns;
using shared::application::Vocabulary;

constexpr const char* SELECT_NODES {R"SQL(
    SELECT id, label, is_hub, zone, x, y FROM road_nodes ORDER BY position
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
                                      SqlColumns::term(Vocabulary::zone(SqlColumns::text(row, 3)), "zone"),
                                      SqlColumns::decimal(row, 4),
                                      SqlColumns::decimal(row, 5)});
    }
    for (const auto& row : m_connection.query(SELECT_EDGES))
    {
        map.connect(SqlColumns::text(row, 0), SqlColumns::text(row, 1), SqlColumns::decimal(row, 2));
    }
}

} // namespace hubflow::routing::adapters
