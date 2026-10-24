#include "bootstrap/modules/logisticsModule.hpp"

#include "bootstrap/datasetSeeder.hpp"

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/logistics/adapters/in/http/catalogController.hpp"
#include "hubflow/logistics/adapters/in/http/dispatchController.hpp"
#include "hubflow/logistics/adapters/in/http/movementController.hpp"
#include "hubflow/logistics/adapters/in/http/operationsController.hpp"
#include "hubflow/logistics/adapters/in/http/shipmentController.hpp"
#include "hubflow/logistics/adapters/in/http/shipmentGroupController.hpp"
#include "hubflow/logistics/adapters/out/generation/publicTrackingCodeGenerator.hpp"
#include "hubflow/logistics/adapters/out/generation/sequentialPkgCodeGenerator.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlCatalogRepository.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentGroupRepository.hpp"
#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentRepository.hpp"
#include "hubflow/logistics/adapters/out/routing/roadMapDeliveryPoints.hpp"
#include "hubflow/logistics/adapters/out/routing/routingPlantOccupancy.hpp"
#include "hubflow/logistics/application/hydration/logisticsHydrator.hpp"
#include "hubflow/logistics/application/subscribers/catalogPersistenceSubscriber.hpp"
#include "hubflow/logistics/application/subscribers/pendingShipmentsProjector.hpp"
#include "hubflow/logistics/application/subscribers/shipmentGroupPersistenceSubscriber.hpp"
#include "hubflow/logistics/application/subscribers/shipmentPersistenceSubscriber.hpp"
#include "hubflow/logistics/application/usecases/commands/classifyShipment.hpp"
#include "hubflow/logistics/application/usecases/commands/createServiceLevel.hpp"
#include "hubflow/logistics/application/usecases/commands/createShipmentFeature.hpp"
#include "hubflow/logistics/application/usecases/commands/createZone.hpp"
#include "hubflow/logistics/application/usecases/commands/deliverShipment.hpp"
#include "hubflow/logistics/application/usecases/commands/dispatchNextShipment.hpp"
#include "hubflow/logistics/application/usecases/commands/dissolveShipmentGroup.hpp"
#include "hubflow/logistics/application/usecases/commands/formShipmentGroup.hpp"
#include "hubflow/logistics/application/usecases/commands/registerShipment.hpp"
#include "hubflow/logistics/application/usecases/commands/rescheduleShipment.hpp"
#include "hubflow/logistics/application/usecases/commands/updateServiceLevel.hpp"
#include "hubflow/logistics/application/usecases/commands/updateShipmentFeature.hpp"
#include "hubflow/logistics/application/usecases/commands/updateZone.hpp"
#include "hubflow/logistics/application/usecases/queries/findHeaviestShipment.hpp"
#include "hubflow/logistics/application/usecases/queries/findShipment.hpp"
#include "hubflow/logistics/application/usecases/queries/findShipmentGroupOfShipment.hpp"
#include "hubflow/logistics/application/usecases/queries/getOperationsMetrics.hpp"
#include "hubflow/logistics/application/usecases/queries/getShipment.hpp"
#include "hubflow/logistics/application/usecases/queries/getShipmentGroup.hpp"
#include "hubflow/logistics/application/usecases/queries/listPendingShipments.hpp"
#include "hubflow/logistics/application/usecases/queries/listServiceLevels.hpp"
#include "hubflow/logistics/application/usecases/queries/listShipmentFeatures.hpp"
#include "hubflow/logistics/application/usecases/queries/listShipmentGroups.hpp"
#include "hubflow/logistics/application/usecases/queries/listShipments.hpp"
#include "hubflow/logistics/application/usecases/queries/listZones.hpp"
#include "hubflow/logistics/application/usecases/queries/summarizeZone.hpp"
#include "hubflow/platform/config/settings.hpp"
#include "hubflow/platform/http/apiRouter.hpp"

#include <vector>

namespace hubflow::bootstrap
{
    namespace
    {
        using namespace logistics;

        std::unique_ptr<application::TrackingCodeGenerator> trackingCodeGeneratorFor(const platform::Settings& settings)
        {
            std::unique_ptr<application::TrackingCodeGenerator> spGenerator;
            if (settings.trackingCodes == "sequential")
            {
                spGenerator = std::make_unique<adapters::SequentialPkgCodeGenerator>();
            }
            else
            {
                spGenerator = std::make_unique<adapters::PublicTrackingCodeGenerator>();
            }
            return spGenerator;
        }
    } // namespace

    struct LogisticsModule::Parts
    {
        Parts(const SharedServices& shared, application::LogisticsModel& model, const LogisticsNeighbours& neighbours)
            : model {model}
            , shipments {shared.connection}
            , shipmentGroups {shared.connection}
            , catalogs {shared.connection}
            , deliveryPoints {neighbours.routing.map}
            , occupancy {model, neighbours.routing}
            , spTrackingCodes {trackingCodeGeneratorFor(shared.settings)}
            , projector {model}
            , shipmentPersistence {shipments}
            , shipmentGroupPersistence {shipmentGroups}
            , catalogPersistence {catalogs}
            , registerShipment {model, deliveryPoints, shared.identities, *spTrackingCodes, shared.bus, shared.clock}
            , classifyShipment {model, shared.bus, shared.clock}
            , rescheduleShipment {model, shared.bus, shared.clock}
            , deliverShipment {model, shared.bus, shared.clock}
            , dispatchNextShipment {model, shared.bus, shared.clock}
            , listPendingShipments {model}
            , formShipmentGroup {model, shared.identities, shared.bus, shared.clock}
            , dissolveShipmentGroup {model, shared.bus, shared.clock}
            , listShipmentGroups {model}
            , getShipmentGroup {model}
            , findShipmentGroupOfShipment {model}
            , listShipments {model}
            , findShipment {model}
            , getShipment {model}
            , summarizeZone {model}
            , findHeaviestShipment {model}
            , getOperationsMetrics {model}
            , createZone {model, shared.bus, shared.clock}
            , updateZone {model, occupancy, shared.bus, shared.clock}
            , listZones {model}
            , createServiceLevel {model, shared.bus, shared.clock}
            , updateServiceLevel {model, occupancy, shared.bus, shared.clock}
            , listServiceLevels {model}
            , createShipmentFeature {model, shared.bus, shared.clock}
            , updateShipmentFeature {model, shared.bus, shared.clock}
            , listShipmentFeatures {model}
            , seeder {registerShipment,
                      ShipmentSeedCollaborators {neighbours.users, neighbours.policy, model, shared.clock}}
        {
            subscribers.add(SubscriptionPhase::Projection, projector);
            subscribers.add(SubscriptionPhase::Persistence, shipmentPersistence);
            subscribers.add(SubscriptionPhase::Persistence, shipmentGroupPersistence);
            subscribers.add(SubscriptionPhase::Persistence, catalogPersistence);
        }

        application::LogisticsModel& model;
        adapters::MySqlShipmentRepository shipments;
        adapters::MySqlShipmentGroupRepository shipmentGroups;
        adapters::MySqlCatalogRepository catalogs;
        adapters::RoadMapDeliveryPoints deliveryPoints;
        adapters::RoutingPlantOccupancy occupancy;
        std::unique_ptr<application::TrackingCodeGenerator> spTrackingCodes;
        application::PendingShipmentsProjector projector;
        application::ShipmentPersistenceSubscriber shipmentPersistence;
        application::ShipmentGroupPersistenceSubscriber shipmentGroupPersistence;
        application::CatalogPersistenceSubscriber catalogPersistence;
        application::RegisterShipment registerShipment;
        application::ClassifyShipment classifyShipment;
        application::RescheduleShipment rescheduleShipment;
        application::DeliverShipment deliverShipment;
        application::DispatchNextShipment dispatchNextShipment;
        application::ListPendingShipments listPendingShipments;
        application::FormShipmentGroup formShipmentGroup;
        application::DissolveShipmentGroup dissolveShipmentGroup;
        application::ListShipmentGroups listShipmentGroups;
        application::GetShipmentGroup getShipmentGroup;
        application::FindShipmentGroupOfShipment findShipmentGroupOfShipment;
        application::ListShipments listShipments;
        application::FindShipment findShipment;
        application::GetShipment getShipment;
        application::SummarizeZone summarizeZone;
        application::FindHeaviestShipment findHeaviestShipment;
        application::GetOperationsMetrics getOperationsMetrics;
        application::CreateZone createZone;
        application::UpdateZone updateZone;
        application::ListZones listZones;
        application::CreateServiceLevel createServiceLevel;
        application::UpdateServiceLevel updateServiceLevel;
        application::ListServiceLevels listServiceLevels;
        application::CreateShipmentFeature createShipmentFeature;
        application::UpdateShipmentFeature updateShipmentFeature;
        application::ListShipmentFeatures listShipmentFeatures;
        ShipmentSeeder seeder;
        PhasedSubscribers subscribers;
        std::vector<std::unique_ptr<platform::Controller>> controllers;
    };

    LogisticsModule::LogisticsModule(const SharedServices& shared,
                                     logistics::application::LogisticsModel& model,
                                     const LogisticsNeighbours& neighbours)
        : m_spParts {std::make_unique<Parts>(shared, model, neighbours)}
    {
    }

    LogisticsModule::~LogisticsModule() = default;

    void LogisticsModule::subscribe(const SubscriptionPhase phase, shared::application::EventBus& bus)
    {
        m_spParts->subscribers.subscribe(phase, bus);
    }

    void LogisticsModule::hydrate()
    {
        Parts& parts = *m_spParts;
        application::LogisticsHydrator {parts.model}.hydrate(parts.catalogs, parts.shipments, parts.shipmentGroups);
    }

    void LogisticsModule::seed()
    {
        m_spParts->seeder.seedShipments();
    }

    void LogisticsModule::registerRoutes(platform::ApiRouter& router)
    {
        Parts& parts = *m_spParts;
        parts.controllers.push_back(std::make_unique<adapters::ShipmentController>(
            adapters::ShipmentUseCases {parts.registerShipment,
                                        parts.listShipments,
                                        parts.findShipment,
                                        parts.getShipment,
                                        parts.findShipmentGroupOfShipment}));
        parts.controllers.push_back(std::make_unique<adapters::ShipmentGroupController>(
            adapters::ShipmentGroupUseCases {parts.formShipmentGroup,
                                             parts.dissolveShipmentGroup,
                                             parts.listShipmentGroups,
                                             parts.getShipmentGroup}));
        parts.controllers.push_back(std::make_unique<adapters::MovementController>(adapters::MovementUseCases {
            parts.classifyShipment, parts.rescheduleShipment, parts.deliverShipment, parts.getShipment}));
        parts.controllers.push_back(
            std::make_unique<adapters::DispatchController>(parts.dispatchNextShipment, parts.listPendingShipments));
        parts.controllers.push_back(std::make_unique<adapters::OperationsController>(adapters::OperationsUseCases {
            parts.summarizeZone, parts.findHeaviestShipment, parts.getOperationsMetrics}));
        parts.controllers.push_back(
            std::make_unique<adapters::CatalogController>(adapters::CatalogUseCases {parts.createZone,
                                                                                     parts.updateZone,
                                                                                     parts.listZones,
                                                                                     parts.createServiceLevel,
                                                                                     parts.updateServiceLevel,
                                                                                     parts.listServiceLevels,
                                                                                     parts.createShipmentFeature,
                                                                                     parts.updateShipmentFeature,
                                                                                     parts.listShipmentFeatures}));
        for (auto& spController : parts.controllers)
        {
            spController->registerRoutes(router);
        }
    }

} // namespace hubflow::bootstrap
