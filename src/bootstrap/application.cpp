#include "bootstrap/application.hpp"

#include "bootstrap/datasetSeeder.hpp"
#include "bootstrap/shutdownSignal.hpp"

#include "hubflow/audit/adapters/in/http/auditController.hpp"
#include "hubflow/audit/adapters/out/mysql/mySqlAuditLog.hpp"
#include "hubflow/audit/application/ports/in/countAuditEventsUseCase.hpp"
#include "hubflow/audit/application/ports/in/listAuditEventsUseCase.hpp"
#include "hubflow/audit/application/subscribers/auditLogSubscriber.hpp"
#include "hubflow/audit/application/usecases/queries/countAuditEvents.hpp"
#include "hubflow/audit/application/usecases/queries/listAuditEvents.hpp"
#include "hubflow/identity/adapters/in/http/sessionController.hpp"
#include "hubflow/identity/adapters/in/http/userController.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlRolePermissionRepository.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlUserRepository.hpp"
#include "hubflow/identity/adapters/out/routing/routingCourierWorkload.hpp"
#include "hubflow/identity/adapters/out/security/argon2PasswordHasher.hpp"
#include "hubflow/identity/adapters/out/security/jwtTokenIssuer.hpp"
#include "hubflow/identity/application/hydration/accessPolicyHydrator.hpp"
#include "hubflow/identity/application/ports/in/authenticateUseCase.hpp"
#include "hubflow/identity/application/ports/in/findUserUseCase.hpp"
#include "hubflow/identity/application/ports/in/listCouriersUseCase.hpp"
#include "hubflow/identity/application/ports/in/listUsersUseCase.hpp"
#include "hubflow/identity/application/ports/in/registerUserUseCase.hpp"
#include "hubflow/identity/application/ports/in/signInUseCase.hpp"
#include "hubflow/identity/application/ports/in/updateUserUseCase.hpp"
#include "hubflow/identity/application/subscribers/userPersistenceSubscriber.hpp"
#include "hubflow/identity/application/usecases/commands/registerUser.hpp"
#include "hubflow/identity/application/usecases/commands/signIn.hpp"
#include "hubflow/identity/application/usecases/commands/updateUser.hpp"
#include "hubflow/identity/application/usecases/queries/authenticate.hpp"
#include "hubflow/identity/application/usecases/queries/findUser.hpp"
#include "hubflow/identity/application/usecases/queries/listCouriers.hpp"
#include "hubflow/identity/application/usecases/queries/listUsers.hpp"
#include "hubflow/insights/adapters/in/http/insightController.hpp"
#include "hubflow/insights/application/ports/in/combineCodeSetsUseCase.hpp"
#include "hubflow/insights/application/ports/in/compareShipmentStatesUseCase.hpp"
#include "hubflow/insights/application/ports/in/getBPlusTreeUseCase.hpp"
#include "hubflow/insights/application/ports/in/getBstUseCase.hpp"
#include "hubflow/insights/application/ports/in/getCodeSetUseCase.hpp"
#include "hubflow/insights/application/ports/in/lookupInBPlusTreeUseCase.hpp"
#include "hubflow/insights/application/ports/in/lookupInBstUseCase.hpp"
#include "hubflow/insights/application/ports/in/rangeInBPlusTreeUseCase.hpp"
#include "hubflow/insights/application/usecases/queries/combineCodeSets.hpp"
#include "hubflow/insights/application/usecases/queries/compareShipmentStates.hpp"
#include "hubflow/insights/application/usecases/queries/getBPlusTree.hpp"
#include "hubflow/insights/application/usecases/queries/getBst.hpp"
#include "hubflow/insights/application/usecases/queries/getCodeSet.hpp"
#include "hubflow/insights/application/usecases/queries/lookupInBPlusTree.hpp"
#include "hubflow/insights/application/usecases/queries/lookupInBst.hpp"
#include "hubflow/insights/application/usecases/queries/rangeInBPlusTree.hpp"
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
#include "hubflow/logistics/application/ports/in/classifyShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/createServiceLevelUseCase.hpp"
#include "hubflow/logistics/application/ports/in/createShipmentFeatureUseCase.hpp"
#include "hubflow/logistics/application/ports/in/createZoneUseCase.hpp"
#include "hubflow/logistics/application/ports/in/deliverShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/dispatchNextShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/dissolveShipmentGroupUseCase.hpp"
#include "hubflow/logistics/application/ports/in/findHeaviestShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/findShipmentGroupOfShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/findShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/formShipmentGroupUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getOperationsMetricsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentGroupUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listPendingShipmentsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listServiceLevelsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentFeaturesUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentGroupsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listZonesUseCase.hpp"
#include "hubflow/logistics/application/ports/in/registerShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/rescheduleShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/summarizeZoneUseCase.hpp"
#include "hubflow/logistics/application/ports/in/updateServiceLevelUseCase.hpp"
#include "hubflow/logistics/application/ports/in/updateShipmentFeatureUseCase.hpp"
#include "hubflow/logistics/application/ports/in/updateZoneUseCase.hpp"
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
#include "hubflow/platform/http/apiRouter.hpp"
#include "hubflow/platform/http/healthController.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/routing/adapters/in/http/roadMapController.hpp"
#include "hubflow/routing/adapters/in/http/routeController.hpp"
#include "hubflow/routing/adapters/out/export/failureTolerantRoutePlanExporter.hpp"
#include "hubflow/routing/adapters/out/export/jsonFileRoutePlanExporter.hpp"
#include "hubflow/routing/adapters/out/logistics/logisticsStopShipments.hpp"
#include "hubflow/routing/adapters/out/logistics/logisticsZoneDirectory.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRoadMapRepository.hpp"
#include "hubflow/routing/adapters/out/mysql/mySqlRouteRepository.hpp"
#include "hubflow/routing/application/hydration/routingHydrator.hpp"
#include "hubflow/routing/application/ports/in/addRoadNodeUseCase.hpp"
#include "hubflow/routing/application/ports/in/assignDeliveryRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/confirmStopParcelUseCase.hpp"
#include "hubflow/routing/application/ports/in/getCourierRouteSheetUseCase.hpp"
#include "hubflow/routing/application/ports/in/getRoadMapUseCase.hpp"
#include "hubflow/routing/application/ports/in/getRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/listRoutesUseCase.hpp"
#include "hubflow/routing/application/ports/in/previewRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/reassignDeliveryRouteUseCase.hpp"
#include "hubflow/routing/application/ports/in/resolveRouteStopUseCase.hpp"
#include "hubflow/routing/application/ports/in/updateRoadNodeUseCase.hpp"
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
#include "hubflow/shared/adapters/out/generation/uuidV7IdentityGenerator.hpp"
#include "hubflow/shared/adapters/out/memory/slidingWindowRateLimiter.hpp"
#include "hubflow/shared/adapters/out/mysql/mySqlIdempotencyStore.hpp"
#include "hubflow/shared/adapters/out/system/systemClock.hpp"
#include "hubflow/sorting/adapters/in/http/sortingBatchController.hpp"
#include "hubflow/sorting/application/ports/in/binarySearchInSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/compareSortingBatchSearchesUseCase.hpp"
#include "hubflow/sorting/application/ports/in/getActiveSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/getSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/linearSearchInSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/openSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/prepareSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/processSortingBatchItemUseCase.hpp"
#include "hubflow/sorting/application/ports/in/returnProcessedItemUseCase.hpp"
#include "hubflow/sorting/application/ports/in/sortSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/usecases/commands/openSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/commands/prepareSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/commands/processSortingBatchItem.hpp"
#include "hubflow/sorting/application/usecases/commands/returnProcessedItem.hpp"
#include "hubflow/sorting/application/usecases/commands/sortSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/binarySearchInSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/compareSortingBatchSearches.hpp"
#include "hubflow/sorting/application/usecases/queries/getActiveSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/getSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/linearSearchInSortingBatch.hpp"
#include "hubflow/tracking/adapters/in/http/customerNoticeController.hpp"
#include "hubflow/tracking/adapters/in/http/trackingController.hpp"
#include "hubflow/tracking/adapters/out/logistics/plantTrackedShipments.hpp"
#include "hubflow/tracking/adapters/out/mysql/mySqlCustomerNoticeLog.hpp"
#include "hubflow/tracking/adapters/out/notification/compositeCustomerNotifier.hpp"
#include "hubflow/tracking/adapters/out/notification/logCustomerNotifier.hpp"
#include "hubflow/tracking/adapters/out/notification/recordingCustomerNotifier.hpp"
#include "hubflow/tracking/adapters/out/routing/routingDeliveryProgress.hpp"
#include "hubflow/tracking/adapters/out/security/hmacTrackingTokenIssuer.hpp"
#include "hubflow/tracking/application/subscribers/customerNoticeSubscriber.hpp"
#include "hubflow/tracking/application/usecases/commands/verifyTracking.hpp"
#include "hubflow/tracking/application/usecases/queries/getPublicTracking.hpp"
#include "hubflow/tracking/application/usecases/queries/getVerifiedTracking.hpp"
#include "hubflow/tracking/application/usecases/queries/listCustomerNotices.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace hubflow::bootstrap
{
namespace
{
constexpr std::chrono::milliseconds POLL_INTERVAL {100};
constexpr std::size_t               VERIFICATION_ATTEMPTS {5};
constexpr std::int64_t              VERIFICATION_WINDOW_SECONDS {15 * 60};

std::unique_ptr<logistics::application::TrackingCodeGenerator>
trackingCodeGeneratorFor(const platform::Settings& settings)
{
    std::unique_ptr<logistics::application::TrackingCodeGenerator> spGenerator;
    if (settings.trackingCodes == "sequential")
    {
        spGenerator = std::make_unique<logistics::adapters::SequentialPkgCodeGenerator>();
    }
    else
    {
        spGenerator = std::make_unique<logistics::adapters::PublicTrackingCodeGenerator>();
    }
    return spGenerator;
}
} // namespace

struct Application::Parts
{
    explicit Parts(platform::Settings configuration)
        : settings {std::move(configuration)}
        , connection {settings.database}
        , tokens {settings.jwtSecret}
        , fileExporter {settings.exportDirectory}
        , exporter {fileExporter}
        , deliveryPoints {routing.map}
        , occupancy {logistics, routing}
        , zoneDirectory {logistics}
        , courierWorkload {routing}
        , shipments {connection}
        , shipmentGroups {connection}
        , catalogs {connection}
        , routes {connection}
        , users {connection}
        , rolePermissions {connection}
        , maps {connection}
        , audit {connection}
        , idempotency {connection}
        , noticeLog {connection}
        , trackingTokens {settings.jwtSecret}
        , verificationLimiter {VERIFICATION_ATTEMPTS, VERIFICATION_WINDOW_SECONDS}
        , trackedShipments {logistics, routing}
        , deliveryProgress {routing}
        , recordingNotifier {noticeLog}
        , logNotifier {std::cout}
        , m_projector {logistics}
        , m_shipmentPersistence {shipments}
        , m_shipmentGroupPersistence {shipmentGroups}
        , m_routePersistence {routes}
        , m_userPersistence {users}
        , m_catalogPersistence {catalogs}
        , m_roadMapPersistence {maps}
        , m_routeExport {routing, exporter}
        , m_customerNotices {customerNotifier, settings.trackingLinkBase()}
        , m_auditLog {audit}
        , m_signIn {users, hasher, tokens, clock}
        , m_authenticate {users, tokens, policy, clock}
        , m_registerUser {users, hasher, bus, clock}
        , m_updateUser {users, hasher, courierWorkload, bus, clock}
        , m_listUsers {users}
        , m_listCouriers {users}
        , m_findUser {users}
        , m_listAuditEvents {audit}
        , m_countAuditEvents {audit}
        , spTrackingCodes {trackingCodeGeneratorFor(settings)}
        , m_registerShipment {logistics, deliveryPoints, identities, *spTrackingCodes, bus, clock}
        , m_classifyShipment {logistics, bus, clock}
        , m_rescheduleShipment {logistics, bus, clock}
        , m_deliverShipment {logistics, bus, clock}
        , m_dispatchNextShipment {logistics, bus, clock}
        , m_listPendingShipments {logistics}
        , m_formShipmentGroup {logistics, identities, bus, clock}
        , m_dissolveShipmentGroup {logistics, bus, clock}
        , m_listShipmentGroups {logistics}
        , m_getShipmentGroup {logistics}
        , m_findShipmentGroupOfShipment {logistics}
        , m_listShipments {logistics}
        , m_findShipment {logistics}
        , m_getShipment {logistics}
        , m_summarizeZone {logistics}
        , m_findHeaviestShipment {logistics}
        , m_getOperationsMetrics {logistics}
        , m_createZone {logistics, bus, clock}
        , m_updateZone {logistics, occupancy, bus, clock}
        , m_listZones {logistics}
        , m_createServiceLevel {logistics, bus, clock}
        , m_updateServiceLevel {logistics, occupancy, bus, clock}
        , m_listServiceLevels {logistics}
        , m_createShipmentFeature {logistics, bus, clock}
        , m_updateShipmentFeature {logistics, bus, clock}
        , m_listShipmentFeatures {logistics}
        , m_openSortingBatch {logistics, desk}
        , m_sortSortingBatch {desk}
        , m_prepareSortingBatch {desk}
        , m_processSortingBatchItem {desk, logistics.groups}
        , m_returnProcessedItem {desk, logistics.groups}
        , m_getActiveSortingBatch {desk}
        , m_getSortingBatch {desk}
        , m_linearSearchInSortingBatch {desk}
        , m_binarySearchInSortingBatch {desk}
        , m_compareSortingBatchSearches {desk}
        , m_assignDeliveryRoute {routing, desk, logistics.groups, users, bus, clock}
        , m_resolveRouteStop {logistics, routing, bus, clock}
        , m_confirmStopParcel {logistics, routing, bus, clock}
        , m_reassignDeliveryRoute {routing, users, bus, clock}
        , m_addRoadNode {routing, zoneDirectory, bus, clock}
        , m_updateRoadNode {routing, zoneDirectory, bus, clock}
        , m_getRoadMap {routing}
        , m_previewRoute {routing}
        , m_listRoutes {routing}
        , m_stopShipments {logistics}
        , m_getRoute {routing}
        , m_getCourierRouteSheet {m_getRoute, m_stopShipments}
        , m_lookupInBst {logistics}
        , m_getBst {logistics}
        , m_lookupInBPlusTree {logistics}
        , m_rangeInBPlusTree {logistics}
        , m_getBPlusTree {logistics}
        , m_compareShipmentStates {logistics}
        , m_getCodeSet {logistics}
        , m_combineCodeSets {logistics}
        , m_getPublicTracking {trackedShipments}
        , m_verifyTracking {trackedShipments, verificationLimiter, trackingTokens, clock}
        , m_getVerifiedTracking {trackedShipments, deliveryProgress, trackingTokens, clock}
        , m_listCustomerNotices {noticeLog}
        , seeder {m_registerUser, users, m_registerShipment, logistics, policy, clock, settings.seedPassword}
    {
        bus.subscribe(m_projector);
        bus.subscribe(m_shipmentPersistence);
        bus.subscribe(m_shipmentGroupPersistence);
        bus.subscribe(m_routePersistence);
        bus.subscribe(m_userPersistence);
        bus.subscribe(m_catalogPersistence);
        bus.subscribe(m_roadMapPersistence);
        bus.subscribe(m_routeExport);
        bus.subscribe(m_customerNotices);
        bus.subscribe(m_auditLog);
        identity::application::AccessPolicyHydrator {policy}.hydrate(rolePermissions);
        logistics::application::LogisticsHydrator {logistics}.hydrate(catalogs, shipments, shipmentGroups);
        routing::application::RoutingHydrator {routing}.hydrate(maps, routes);
        customerNotifier.add(logNotifier);
        customerNotifier.add(recordingNotifier);
        seeder.seedUsers();
        seeder.seedShipments();
        router = std::make_unique<platform::ApiRouter>(server, m_authenticate, idempotency, mutex);
        wire();
    }

    void wire();

    platform::Settings                                             settings;
    platform::MySqlConnection                                      connection;
    shared::adapters::SystemClock                                  clock;
    identity::adapters::Argon2PasswordHasher                       hasher;
    identity::adapters::JwtTokenIssuer                             tokens;
    routing::adapters::JsonFileRoutePlanExporter                   fileExporter;
    routing::adapters::FailureTolerantRoutePlanExporter            exporter;
    shared::application::EventBus                                  bus;
    shared::adapters::UuidV7IdentityGenerator                      identities;
    logistics::application::LogisticsModel                         logistics;
    routing::application::RoutingModel                             routing;
    sorting::domain::SortingDesk                                   desk;
    logistics::adapters::RoadMapDeliveryPoints                     deliveryPoints;
    logistics::adapters::RoutingPlantOccupancy                     occupancy;
    routing::adapters::LogisticsZoneDirectory                      zoneDirectory;
    identity::adapters::RoutingCourierWorkload                     courierWorkload;
    identity::domain::AccessPolicy                                 policy;
    logistics::adapters::MySqlShipmentRepository                   shipments;
    logistics::adapters::MySqlShipmentGroupRepository              shipmentGroups;
    logistics::adapters::MySqlCatalogRepository                    catalogs;
    routing::adapters::MySqlRouteRepository                        routes;
    identity::adapters::MySqlUserRepository                        users;
    identity::adapters::MySqlRolePermissionRepository              rolePermissions;
    routing::adapters::MySqlRoadMapRepository                      maps;
    audit::adapters::MySqlAuditLog                                 audit;
    shared::adapters::MySqlIdempotencyStore                        idempotency;
    tracking::adapters::MySqlCustomerNoticeLog                     noticeLog;
    tracking::adapters::HmacTrackingTokenIssuer                    trackingTokens;
    shared::adapters::SlidingWindowRateLimiter                     verificationLimiter;
    tracking::adapters::PlantTrackedShipments                      trackedShipments;
    tracking::adapters::RoutingDeliveryProgress                    deliveryProgress;
    tracking::adapters::RecordingCustomerNotifier                  recordingNotifier;
    tracking::adapters::LogCustomerNotifier                        logNotifier;
    tracking::adapters::CompositeCustomerNotifier                  customerNotifier;
    logistics::application::PendingShipmentsProjector              m_projector;
    logistics::application::ShipmentPersistenceSubscriber          m_shipmentPersistence;
    logistics::application::ShipmentGroupPersistenceSubscriber     m_shipmentGroupPersistence;
    routing::application::RoutePersistenceSubscriber               m_routePersistence;
    identity::application::UserPersistenceSubscriber               m_userPersistence;
    logistics::application::CatalogPersistenceSubscriber           m_catalogPersistence;
    routing::application::RoadMapPersistenceSubscriber             m_roadMapPersistence;
    routing::application::RoutePlanExportSubscriber                m_routeExport;
    tracking::application::CustomerNoticeSubscriber                m_customerNotices;
    audit::application::AuditLogSubscriber                         m_auditLog;
    identity::application::SignIn                                  m_signIn;
    identity::application::Authenticate                            m_authenticate;
    identity::application::RegisterUser                            m_registerUser;
    identity::application::UpdateUser                              m_updateUser;
    identity::application::ListUsers                               m_listUsers;
    identity::application::ListCouriers                            m_listCouriers;
    identity::application::FindUser                                m_findUser;
    audit::application::ListAuditEvents                            m_listAuditEvents;
    audit::application::CountAuditEvents                           m_countAuditEvents;
    std::unique_ptr<logistics::application::TrackingCodeGenerator> spTrackingCodes;
    logistics::application::RegisterShipment                       m_registerShipment;
    logistics::application::ClassifyShipment                       m_classifyShipment;
    logistics::application::RescheduleShipment                     m_rescheduleShipment;
    logistics::application::DeliverShipment                        m_deliverShipment;
    logistics::application::DispatchNextShipment                   m_dispatchNextShipment;
    logistics::application::ListPendingShipments                   m_listPendingShipments;
    logistics::application::FormShipmentGroup                      m_formShipmentGroup;
    logistics::application::DissolveShipmentGroup                  m_dissolveShipmentGroup;
    logistics::application::ListShipmentGroups                     m_listShipmentGroups;
    logistics::application::GetShipmentGroup                       m_getShipmentGroup;
    logistics::application::FindShipmentGroupOfShipment            m_findShipmentGroupOfShipment;
    logistics::application::ListShipments                          m_listShipments;
    logistics::application::FindShipment                           m_findShipment;
    logistics::application::GetShipment                            m_getShipment;
    logistics::application::SummarizeZone                          m_summarizeZone;
    logistics::application::FindHeaviestShipment                   m_findHeaviestShipment;
    logistics::application::GetOperationsMetrics                   m_getOperationsMetrics;
    logistics::application::CreateZone                             m_createZone;
    logistics::application::UpdateZone                             m_updateZone;
    logistics::application::ListZones                              m_listZones;
    logistics::application::CreateServiceLevel                     m_createServiceLevel;
    logistics::application::UpdateServiceLevel                     m_updateServiceLevel;
    logistics::application::ListServiceLevels                      m_listServiceLevels;
    logistics::application::CreateShipmentFeature                  m_createShipmentFeature;
    logistics::application::UpdateShipmentFeature                  m_updateShipmentFeature;
    logistics::application::ListShipmentFeatures                   m_listShipmentFeatures;
    sorting::application::OpenSortingBatch                         m_openSortingBatch;
    sorting::application::SortSortingBatch                         m_sortSortingBatch;
    sorting::application::PrepareSortingBatch                      m_prepareSortingBatch;
    sorting::application::ProcessSortingBatchItem                  m_processSortingBatchItem;
    sorting::application::ReturnProcessedItem                      m_returnProcessedItem;
    sorting::application::GetActiveSortingBatch                    m_getActiveSortingBatch;
    sorting::application::GetSortingBatch                          m_getSortingBatch;
    sorting::application::LinearSearchInSortingBatch               m_linearSearchInSortingBatch;
    sorting::application::BinarySearchInSortingBatch               m_binarySearchInSortingBatch;
    sorting::application::CompareSortingBatchSearches              m_compareSortingBatchSearches;
    routing::application::AssignDeliveryRoute                      m_assignDeliveryRoute;
    routing::application::ResolveRouteStop                         m_resolveRouteStop;
    routing::application::ConfirmStopParcel                        m_confirmStopParcel;
    routing::application::ReassignDeliveryRoute                    m_reassignDeliveryRoute;
    routing::application::AddRoadNode                              m_addRoadNode;
    routing::application::UpdateRoadNode                           m_updateRoadNode;
    routing::application::GetRoadMap                               m_getRoadMap;
    routing::application::PreviewRoute                             m_previewRoute;
    routing::application::ListRoutes                               m_listRoutes;
    routing::adapters::LogisticsStopShipments                      m_stopShipments;
    routing::application::GetRoute                                 m_getRoute;
    routing::application::GetCourierRouteSheet                     m_getCourierRouteSheet;
    insights::application::LookupInBst                             m_lookupInBst;
    insights::application::GetBst                                  m_getBst;
    insights::application::LookupInBPlusTree                       m_lookupInBPlusTree;
    insights::application::RangeInBPlusTree                        m_rangeInBPlusTree;
    insights::application::GetBPlusTree                            m_getBPlusTree;
    insights::application::CompareShipmentStates                   m_compareShipmentStates;
    insights::application::GetCodeSet                              m_getCodeSet;
    insights::application::CombineCodeSets                         m_combineCodeSets;
    tracking::application::GetPublicTracking                       m_getPublicTracking;
    tracking::application::VerifyTracking                          m_verifyTracking;
    tracking::application::GetVerifiedTracking                     m_getVerifiedTracking;
    tracking::application::ListCustomerNotices                     m_listCustomerNotices;
    DatasetSeeder                                                  seeder;
    httplib::Server                                                server;
    std::mutex                                                     mutex;
    std::unique_ptr<platform::ApiRouter>                           router;
    std::vector<std::unique_ptr<platform::Controller>>             controllers;
};

void Application::Parts::wire()
{
    controllers.push_back(std::make_unique<platform::HealthController>(
        [this]
        {
            connection.query("SELECT 1");
            return true;
        }));
    controllers.push_back(std::make_unique<identity::adapters::SessionController>(m_signIn, m_findUser));
    controllers.push_back(std::make_unique<identity::adapters::UserController>(
        identity::adapters::UserUseCases {m_registerUser, m_updateUser, m_listUsers, m_listCouriers, m_findUser}));
    controllers.push_back(std::make_unique<audit::adapters::AuditController>(m_listAuditEvents, m_countAuditEvents));
    controllers.push_back(
        std::make_unique<logistics::adapters::ShipmentController>(logistics::adapters::ShipmentUseCases {
            m_registerShipment, m_listShipments, m_findShipment, m_getShipment, m_findShipmentGroupOfShipment}));
    controllers.push_back(
        std::make_unique<logistics::adapters::ShipmentGroupController>(logistics::adapters::ShipmentGroupUseCases {
            m_formShipmentGroup, m_dissolveShipmentGroup, m_listShipmentGroups, m_getShipmentGroup}));
    controllers.push_back(
        std::make_unique<logistics::adapters::MovementController>(logistics::adapters::MovementUseCases {
            m_classifyShipment, m_rescheduleShipment, m_deliverShipment, m_getShipment}));
    controllers.push_back(
        std::make_unique<logistics::adapters::DispatchController>(m_dispatchNextShipment, m_listPendingShipments));
    controllers.push_back(std::make_unique<logistics::adapters::OperationsController>(
        logistics::adapters::OperationsUseCases {m_summarizeZone, m_findHeaviestShipment, m_getOperationsMetrics}));
    controllers.push_back(std::make_unique<logistics::adapters::CatalogController>(
        logistics::adapters::CatalogUseCases {m_createZone,
                                              m_updateZone,
                                              m_listZones,
                                              m_createServiceLevel,
                                              m_updateServiceLevel,
                                              m_listServiceLevels,
                                              m_createShipmentFeature,
                                              m_updateShipmentFeature,
                                              m_listShipmentFeatures}));
    controllers.push_back(std::make_unique<sorting::adapters::SortingBatchController>(
        sorting::adapters::SortingUseCases {m_openSortingBatch,
                                            m_sortSortingBatch,
                                            m_prepareSortingBatch,
                                            m_processSortingBatchItem,
                                            m_returnProcessedItem,
                                            m_getActiveSortingBatch,
                                            m_getSortingBatch,
                                            m_linearSearchInSortingBatch,
                                            m_binarySearchInSortingBatch,
                                            m_compareSortingBatchSearches}));
    controllers.push_back(std::make_unique<routing::adapters::RouteController>(
        routing::adapters::RouteUseCases {m_assignDeliveryRoute,
                                          m_resolveRouteStop,
                                          m_confirmStopParcel,
                                          m_reassignDeliveryRoute,
                                          m_listRoutes,
                                          m_getRoute,
                                          m_getRoadMap,
                                          m_getCourierRouteSheet}));
    controllers.push_back(std::make_unique<routing::adapters::RoadMapController>(
        m_getRoadMap, m_previewRoute, m_addRoadNode, m_updateRoadNode));
    controllers.push_back(std::make_unique<insights::adapters::InsightController>(
        insights::adapters::InsightUseCases {m_getBst,
                                             m_lookupInBst,
                                             m_getBPlusTree,
                                             m_lookupInBPlusTree,
                                             m_rangeInBPlusTree,
                                             m_compareShipmentStates,
                                             m_getCodeSet,
                                             m_combineCodeSets}));
    controllers.push_back(std::make_unique<tracking::adapters::TrackingController>(
        tracking::adapters::TrackingUseCases {m_getPublicTracking, m_verifyTracking, m_getVerifiedTracking},
        settings.trackingLinkBase()));
    controllers.push_back(std::make_unique<tracking::adapters::CustomerNoticeController>(m_listCustomerNotices));
    for (auto& spController : controllers)
    {
        spController->registerRoutes(*router);
    }
}

Application::Application(platform::Settings settings)
    : m_spParts {std::make_unique<Parts>(std::move(settings))}
{
}

Application::~Application() = default;

int Application::run()
{
    int              status {0};
    httplib::Server& server   = m_spParts->server;
    const auto&      settings = m_spParts->settings;
    if (std::filesystem::is_directory(settings.webRoot))
    {
        server.set_mount_point("/", settings.webRoot);
    }
    if (!server.bind_to_port(settings.bindAddress, settings.port))
    {
        std::cerr << "[hubflow] no se pudo abrir " << settings.bindAddress << ':' << settings.port << '\n';
        status = 1;
    }
    else
    {
        ShutdownSignal::install();
        std::thread listener {[&server]
                              {
                                  server.listen_after_bind();
                              }};
        std::cout << "[hubflow] escuchando en " << settings.bindAddress << ':' << settings.port << std::endl;
        while (!ShutdownSignal::requested())
        {
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
        std::cout << "[hubflow] apagando" << std::endl;
        server.stop();
        listener.join();
    }
    return status;
}

} // namespace hubflow::bootstrap
