#include "hubflow/routing/application/usecases/queries/getRoute.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::application
{

    GetRoute::GetRoute(const RoutingModel& model)
        : m_model {model}
    {
    }

    const domain::DeliveryRoute& GetRoute::execute(const shared::application::Principal& principal,
                                                   std::uint64_t routeId) const
    {
        principal.require(identity::domain::Permission::ResolveStops);
        const domain::DeliveryRoute& found = m_model.routes.require(routeId);
        if (principal.role == identity::domain::Role::Pum && found.courier().id != principal.userId)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Forbidden, "route.not_owner", "La hoja de ruta pertenece a otro PUM"};
        }
        return found;
    }

} // namespace hubflow::routing::application
