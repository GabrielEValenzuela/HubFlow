#include "hubflow/routing/application/usecases/commands/assignDeliveryRoute.hpp"

#include "datastructures/dynamicArray.hpp"
#include "datastructures/stack.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/routing/application/usecases/requireCourier.hpp"
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

bool isLoaded(const ds::DynamicArray<logistics::domain::Shipment*>& load, const std::string& code)
{
    bool found {false};
    for (std::size_t i = 0; i < load.size() && !found; ++i)
    {
        found = load[i]->code() == code;
    }
    return found;
}

bool isCovered(const ds::DynamicArray<domain::RouteStop>& stops, const std::string& code)
{
    bool found {false};
    for (std::size_t i = 0; i < stops.size() && !found; ++i)
    {
        found = stops[i].includes(code);
    }
    return found;
}

ds::DynamicArray<domain::RouteStop> stopsFrom(const ds::DynamicArray<logistics::domain::Shipment*>& load,
                                              const logistics::domain::ShipmentGroupBook&           groups)
{
    ds::DynamicArray<domain::RouteStop> stops;
    for (std::size_t i = 0; i < load.size(); ++i)
    {
        if (!load[i]->isAwaitingDispatch())
        {
            throw business_error {
                ErrorKind::Conflict, "route.shipment_not_pending", "Un envío del lote ya no está pendiente"};
        }
        if (!isCovered(stops, load[i]->code()))
        {
            domain::RouteStop stop {
                stops.size() + 1, load[i]->code(), load[i]->deliveryPoint(), domain::StopOutcome::Pending};
            const logistics::domain::ShipmentGroup* pGroup = groups.groupOf(load[i]->code());
            for (std::size_t j = 0; pGroup != nullptr && j < pGroup->codes().size(); ++j)
            {
                if (!isLoaded(load, pGroup->codes()[j]))
                {
                    throw business_error {ErrorKind::Conflict,
                                          "route.group_incomplete",
                                          "El paquete " + load[i]->code() + " viaja junto con " + pGroup->codes()[j] +
                                              ", que no está cargado"};
                }
                if (pGroup->codes()[j] != load[i]->code())
                {
                    stop.companions.pushBack(pGroup->codes()[j]);
                }
            }
            stops.pushBack(stop);
        }
    }
    return stops;
}
} // namespace

AssignDeliveryRoute::AssignDeliveryRoute(RoutingModel&                                routing,
                                         sorting::domain::SortingDesk&                desk,
                                         const logistics::domain::ShipmentGroupBook&  groups,
                                         const identity::application::UserRepository& users,
                                         shared::application::EventBus&               bus,
                                         const shared::application::Clock&            clock)
    : m_routing {routing}
    , m_desk {desk}
    , m_groups {groups}
    , m_users {users}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::DeliveryRoute& AssignDeliveryRoute::execute(const shared::application::Principal& principal,
                                                          const AssignRouteRequest&             request)
{
    principal.require(identity::domain::Permission::AssignRoutes);
    const domain::Courier          courier {requireCourier(m_users, request.courierId)};
    sorting::domain::SortingBatch& batch = m_desk.require(request.batchId);
    if (batch.processed().isEmpty())
    {
        throw business_error {
            ErrorKind::Conflict, "route.empty_load", "Procesá al menos un envío del lote antes de asignar la ruta"};
    }
    const auto             context = principal.contextAt(m_clock);
    const domain::PumAgent agent {m_routing.map, domain::SearchStrategy::of(request.algorithm)};
    const auto             load  = loadingOrder(batch.processed());
    domain::DeliveryRoute& route = m_routing.routes.adopt(domain::DeliveryRoute::assign(
        m_routing.routes.nextIdentity(), courier, stopsFrom(load, m_groups), agent, context));
    const std::string      note {"Ruta #" + std::to_string(route.id()) + " asignada a " + courier.name};
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

} // namespace hubflow::routing::application
