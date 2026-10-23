#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROUTE_REPOSITORY_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROUTE_REPOSITORY_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"

namespace hubflow::routing::application
{

/// @brief Receptor de rutas durante la hidratación.
class RouteRestorer
{
public:
    virtual ~RouteRestorer() = default;

    virtual void restoreRoute(const domain::RouteRecord& record) = 0;
};

/// @brief Persistencia de hojas de ruta.
class RouteRepository
{
public:
    virtual ~RouteRepository() = default;

    virtual void add(const domain::RouteRecord& record) = 0;

    virtual void updateStop(std::uint64_t routeId, const domain::RouteStop& stop) = 0;

    /// @brief Registra el nuevo repartidor de la ruta.
    virtual void updateCourier(std::uint64_t routeId, const domain::Courier& courier) = 0;

    virtual void loadAll(RouteRestorer& restorer) const = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ROUTE_REPOSITORY_HPP
