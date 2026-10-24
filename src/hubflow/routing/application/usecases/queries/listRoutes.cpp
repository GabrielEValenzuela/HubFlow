#include "hubflow/routing/application/usecases/queries/listRoutes.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::routing::application
{
    namespace
    {
        bool canSee(const shared::application::Principal& principal, const domain::DeliveryRoute& route)
        {
            return principal.role != identity::domain::Role::Pum || route.courier().id == principal.userId;
        }
    } // namespace

    ListRoutes::ListRoutes(const RoutingModel& model)
        : m_model {model}
    {
    }

    ds::DynamicArray<const domain::DeliveryRoute*>
    ListRoutes::execute(const shared::application::Principal& principal) const
    {
        principal.require(identity::domain::Permission::ResolveStops);
        ds::DynamicArray<const domain::DeliveryRoute*> visible;
        m_model.routes.forEach(
            [&principal, &visible](const domain::DeliveryRoute& route)
            {
                if (canSee(principal, route))
                {
                    visible.pushBack(&route);
                }
            });
        return visible;
    }

} // namespace hubflow::routing::application
