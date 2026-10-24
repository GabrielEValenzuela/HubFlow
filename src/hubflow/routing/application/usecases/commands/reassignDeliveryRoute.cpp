#include "hubflow/routing/application/usecases/commands/reassignDeliveryRoute.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/routing/application/usecases/requireCourier.hpp"

namespace hubflow::routing::application
{

    ReassignDeliveryRoute::ReassignDeliveryRoute(RoutingModel& routing,
                                                 const identity::application::UserRepository& users,
                                                 shared::application::EventBus& bus,
                                                 const shared::application::Clock& clock)
        : m_routing {routing}
        , m_users {users}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::DeliveryRoute& ReassignDeliveryRoute::execute(const shared::application::Principal& principal,
                                                                const std::uint64_t routeId,
                                                                const std::uint64_t courierId)
    {
        principal.require(identity::domain::Permission::AssignRoutes);
        domain::DeliveryRoute& route = m_routing.routes.require(routeId);
        route.reassign(requireCourier(m_users, courierId), principal.contextAt(m_clock));
        route.publishEvents(m_bus);
        return route;
    }

} // namespace hubflow::routing::application
