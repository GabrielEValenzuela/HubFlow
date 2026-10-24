#include "hubflow/identity/adapters/out/routing/routingCourierWorkload.hpp"

namespace hubflow::identity::adapters
{

    RoutingCourierWorkload::RoutingCourierWorkload(const routing::application::RoutingModel& routing) noexcept
        : m_routing {routing}
    {
    }

    bool RoutingCourierWorkload::hasRouteInProgress(const std::uint64_t courierId) const
    {
        bool found {false};
        m_routing.routes.forEach(
            [&found, courierId](const routing::domain::DeliveryRoute& route)
            {
                found = found ||
                        (route.courier().id == courierId && route.status() == routing::domain::RouteStatus::InProgress);
            });
        return found;
    }

} // namespace hubflow::identity::adapters
