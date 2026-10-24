#include "bootstrap/modules/routingModule.hpp"

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/platform/config/settings.hpp"
#include "hubflow/platform/http/apiRouter.hpp"
#include "hubflow/routing/adapters/in/http/roadMapController.hpp"
#include "hubflow/routing/adapters/in/http/routeController.hpp"
#include "hubflow/routing/adapters/out/export/failureTolerantRoutePlanExporter.hpp"
#include "hubflow/routing/adapters/out/export/jsonFileRoutePlanExporter.hpp"
#include "hubflow/routing/adapters/out/logistics/logisticsStopShipments.hpp"
#include "hubflow/routing/adapters/out/logistics/logisticsZoneDirectory.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRoadMapRepository.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRouteRepository.hpp"
#include "hubflow/routing/application/hydration/routingHydrator.hpp"
#include "hubflow/routing/application/subscribers/roadMapPersistenceSubscriber.hpp"
#include "hubflow/routing/application/subscribers/routePersistenceSubscriber.hpp"
#include "hubflow/routing/application/subscribers/routePlanExportSubscriber.hpp"
#include "hubflow/routing/application/usecases/commands/addRoadNode.hpp"
#include "hubflow/routing/application/usecases/commands/assignDeliveryRoute.hpp"
#include "hubflow/routing/application/usecases/commands/confirmStopParcel.hpp"
#include "hubflow/routing/application/usecases/commands/reassignDeliveryRoute.hpp"
#include "hubflow/routing/application/usecases/commands/resolveRouteStop.hpp"
#include "hubflow/routing/application/usecases/commands/updateRoadNode.hpp"
#include "hubflow/routing/application/usecases/queries/getCourierRouteSheet.hpp"
#include "hubflow/routing/application/usecases/queries/getRoadMap.hpp"
#include "hubflow/routing/application/usecases/queries/getRoute.hpp"
#include "hubflow/routing/application/usecases/queries/listRoutes.hpp"
#include "hubflow/routing/application/usecases/queries/previewRoute.hpp"

#include <vector>

namespace hubflow::bootstrap
{
namespace
{
using namespace routing;
} // namespace

struct RoutingModule::Parts
{
    Parts(const SharedServices& shared, application::RoutingModel& model, const RoutingNeighbours& neighbours)
        : model {model}
        , fileExporter {shared.settings.exportDirectory}
        , exporter {fileExporter}
        , zoneDirectory {neighbours.logistics}
        , maps {shared.connection}
        , routes {shared.connection}
        , routePersistence {routes}
        , roadMapPersistence {maps}
        , routeExport {model, exporter}
        , assignDeliveryRoute {model,
                               neighbours.desk,
                               neighbours.logistics.groups,
                               neighbours.users,
                               shared.bus,
                               shared.clock}
        , resolveRouteStop {neighbours.logistics, model, shared.bus, shared.clock}
        , confirmStopParcel {neighbours.logistics, model, shared.bus, shared.clock}
        , reassignDeliveryRoute {model, neighbours.users, shared.bus, shared.clock}
        , addRoadNode {model, zoneDirectory, shared.bus, shared.clock}
        , updateRoadNode {model, zoneDirectory, shared.bus, shared.clock}
        , getRoadMap {model}
        , previewRoute {model}
        , listRoutes {model}
        , stopShipments {neighbours.logistics}
        , getRoute {model}
        , getCourierRouteSheet {getRoute, stopShipments}
    {
        subscribers.add(SubscriptionPhase::Persistence, routePersistence);
        subscribers.add(SubscriptionPhase::Persistence, roadMapPersistence);
        subscribers.add(SubscriptionPhase::Export, routeExport);
    }

    application::RoutingModel&                         model;
    adapters::JsonFileRoutePlanExporter                fileExporter;
    adapters::FailureTolerantRoutePlanExporter         exporter;
    adapters::LogisticsZoneDirectory                   zoneDirectory;
    adapters::MySqlRoadMapRepository                   maps;
    adapters::MySqlRouteRepository                     routes;
    application::RoutePersistenceSubscriber            routePersistence;
    application::RoadMapPersistenceSubscriber          roadMapPersistence;
    application::RoutePlanExportSubscriber             routeExport;
    application::AssignDeliveryRoute                   assignDeliveryRoute;
    application::ResolveRouteStop                      resolveRouteStop;
    application::ConfirmStopParcel                     confirmStopParcel;
    application::ReassignDeliveryRoute                 reassignDeliveryRoute;
    application::AddRoadNode                           addRoadNode;
    application::UpdateRoadNode                        updateRoadNode;
    application::GetRoadMap                            getRoadMap;
    application::PreviewRoute                          previewRoute;
    application::ListRoutes                            listRoutes;
    adapters::LogisticsStopShipments                   stopShipments;
    application::GetRoute                              getRoute;
    application::GetCourierRouteSheet                  getCourierRouteSheet;
    PhasedSubscribers                                  subscribers;
    std::vector<std::unique_ptr<platform::Controller>> controllers;
};

RoutingModule::RoutingModule(const SharedServices&               shared,
                             routing::application::RoutingModel& model,
                             const RoutingNeighbours&            neighbours)
    : m_spParts {std::make_unique<Parts>(shared, model, neighbours)}
{
}

RoutingModule::~RoutingModule() = default;

void RoutingModule::subscribe(const SubscriptionPhase phase, shared::application::EventBus& bus)
{
    m_spParts->subscribers.subscribe(phase, bus);
}

void RoutingModule::hydrate()
{
    Parts& parts = *m_spParts;
    application::RoutingHydrator {parts.model}.hydrate(parts.maps, parts.routes);
}

void RoutingModule::registerRoutes(platform::ApiRouter& router)
{
    Parts& parts = *m_spParts;
    parts.controllers.push_back(
        std::make_unique<adapters::RouteController>(adapters::RouteUseCases {parts.assignDeliveryRoute,
                                                                             parts.resolveRouteStop,
                                                                             parts.confirmStopParcel,
                                                                             parts.reassignDeliveryRoute,
                                                                             parts.listRoutes,
                                                                             parts.getRoute,
                                                                             parts.getRoadMap,
                                                                             parts.getCourierRouteSheet}));
    parts.controllers.push_back(std::make_unique<adapters::RoadMapController>(
        parts.getRoadMap, parts.previewRoute, parts.addRoadNode, parts.updateRoadNode));
    for (auto& spController : parts.controllers)
    {
        spController->registerRoutes(router);
    }
}

} // namespace hubflow::bootstrap
