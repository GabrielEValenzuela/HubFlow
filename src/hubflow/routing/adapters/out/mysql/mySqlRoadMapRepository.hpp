#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_MYSQL_MY_SQL_ROAD_MAP_REPOSITORY_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_MYSQL_MY_SQL_ROAD_MAP_REPOSITORY_HPP

#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/routing/application/ports/out/roadMapRepository.hpp"

namespace hubflow::routing::adapters
{

/// @brief Mapa de la ciudad guardado en las tablas @c road_nodes y @c road_edges.
class MySqlRoadMapRepository final : public application::RoadMapRepository
{
public:
    explicit MySqlRoadMapRepository(platform::MySqlConnection& connection);

private:
    void loadInto(domain::RoadMap& map) const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_MYSQL_MY_SQL_ROAD_MAP_REPOSITORY_HPP
