#include "hubflow/routing/application/usecases/commands/assignDeliveryRoute.hpp"

#include "datastructures/dynamicArray.hpp"
#include "datastructures/stack.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/routing/domain/services/pumAgent.hpp"
#include "hubflow/routing/domain/services/search/searchStrategy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <string>

namespace hubflow::routing::application
{
namespace
{
using shared::domain::business_error;
using shared::domain::ErrorKind;

ds::DynamicArray<logistics::domain::Shipment*> loadingOrder(const ds::Stack<logistics::domain::Shipment*>& processed)
{
    ds::DynamicArray<logistics::domain::Shipment*> load;
    processed.forEach(
        [&load](logistics::domain::Shipment* pShipment)
        {
            load.pushBack(pShipment);
        });
    return load;
}

ds::DynamicArray<domain::RouteStop> stopsFrom(const ds::DynamicArray<logistics::domain::Shipment*>& load)
{
    ds::DynamicArray<domain::RouteStop> stops;
    for (std::size_t i = 0; i < load.size(); ++i)
    {
        if (!load[i]->isAwaitingDispatch())
        {
            throw business_error {
                ErrorKind::Conflict, "route.shipment_not_pending", "Un envío del lote ya no está pendiente"};
        }
        stops.pushBack(
            domain::RouteStop {i + 1, load[i]->code(), load[i]->deliveryPoint(), domain::StopOutcome::Pending});
    }
    return stops;
}
} // namespace

AssignDeliveryRoute::AssignDeliveryRoute(RoutingModel&                                routing,
                                         sorting::domain::SortingDesk&                desk,
                                         const identity::application::UserRepository& users,
                                         shared::application::EventBus&               bus,
                                         const shared::application::Clock&            clock)
    : m_routing {routing}
    , m_desk {desk}
    , m_users {users}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::DeliveryRoute& AssignDeliveryRoute::execute(const shared::application::Principal& principal,
                                                          const AssignRouteRequest&             request)
{
    principal.require(identity::domain::Permission::AssignRoutes);
    const domain::Courier          courier {requireCourier(request.courierId)};
    sorting::domain::SortingBatch& batch = m_desk.require(request.batchId);
    if (batch.processed().isEmpty())
    {
        throw business_error {
            ErrorKind::Conflict, "route.empty_load", "Procesá al menos un envío del lote antes de asignar la ruta"};
    }
    const auto             context = principal.contextAt(m_clock);
    const domain::PumAgent agent {m_routing.map, domain::SearchStrategy::of(request.algorithm)};
    const auto             load  = loadingOrder(batch.processed());
    domain::DeliveryRoute& route = m_routing.routes.adopt(
        domain::DeliveryRoute::assign(m_routing.routes.nextIdentity(), courier, stopsFrom(load), agent, context));
    const std::string note {"Ruta #" + std::to_string(route.id()) + " asignada a " + courier.name};
    for (std::size_t i = 0; i < load.size(); ++i)
    {
        load[i]->dispatch(note, context);
        load[i]->publishEvents(m_bus);
    }
    route.publishEvents(m_bus);
    batch.unloadProcessed();
    m_desk.close();
    return route;
}

domain::Courier AssignDeliveryRoute::requireCourier(const std::uint64_t courierId) const
{
    const auto user = m_users.byId(courierId);
    if (!user.has_value() || !user->active || user->role != identity::domain::Role::Pum)
    {
        throw business_error {ErrorKind::Validation, "route.courier_invalid", "El repartidor no es un PUM activo"};
    }
    return domain::Courier {user->id, user->name};
}

} // namespace hubflow::routing::application
