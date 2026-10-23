#ifndef HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_ROUTE_REPOSITORY_HPP
#define HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_ROUTE_REPOSITORY_HPP

#include "domain/routing/repositories/routeRepository.hpp"
#include "infrastructure/persistence/mysql/mySqlConnection.hpp"

namespace hubflow::infrastructure
{

/// @brief Hojas de ruta en las tablas @c routes y @c route_stops.
class MySqlRouteRepository final : public domain::routing::RouteRepository
{
public:
    explicit MySqlRouteRepository(MySqlConnection& connection);

private:
    void add(const domain::routing::RouteRecord& record) override;
    void updateStop(std::uint64_t routeId, const domain::routing::RouteStop& stop) override;
    void loadAll(domain::routing::RouteRestorer& restorer) const override;

    MySqlConnection& m_connection;
};

/// @brief Mapa de la ciudad en las tablas @c road_nodes y @c road_edges.
class MySqlRoadMapRepository final : public domain::routing::RoadMapRepository
{
public:
    explicit MySqlRoadMapRepository(MySqlConnection& connection);

private:
    void loadInto(domain::routing::RoadMap& map) const override;

    MySqlConnection& m_connection;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_ROUTE_REPOSITORY_HPP
