#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_MYSQL_MY_SQL_ROUTE_REPOSITORY_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_MYSQL_MY_SQL_ROUTE_REPOSITORY_HPP

#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/routing/application/ports/out/routeRepository.hpp"

namespace hubflow::routing::adapters
{

/// @brief Hojas de ruta guardadas en las tablas @c routes y @c route_stops.
class MySqlRouteRepository final : public application::RouteRepository
{
public:
    explicit MySqlRouteRepository(platform::MySqlConnection& connection);

private:
    void add(const domain::RouteRecord& record) override;
    void updateStop(std::uint64_t routeId, const domain::RouteStop& stop) override;
    void loadAll(application::RouteRestorer& restorer) const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_MYSQL_MY_SQL_ROUTE_REPOSITORY_HPP
