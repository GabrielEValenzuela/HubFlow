#include "support/applicationHarness.hpp"

#include "support/fixtures.hpp"

namespace hubflow::test
{

ApplicationHarness::ApplicationHarness(Storage& sharedStorage)
    : storage {sharedStorage}
    , deliveryPoints {routing.map}
    , m_projector {logistics}
    , m_shipmentPersistence {storage.shipments}
    , m_routePersistence {storage.routes}
    , m_userPersistence {storage.users}
    , m_auditLog {storage.audit}
    , m_signIn {storage.users, hasher, tokens, clock}
    , m_authenticate {storage.users, tokens, clock}
    , m_registerUser {storage.users, hasher, bus, clock}
    , m_updateUser {storage.users, hasher, bus, clock}
    , m_listUsers {storage.users}
    , m_listCouriers {storage.users}
    , m_findUser {storage.users}
    , m_listAuditEvents {storage.audit}
    , m_countAuditEvents {storage.audit}
    , m_registerShipment {logistics, deliveryPoints, bus, clock}
    , m_classifyShipment {logistics, bus, clock}
    , m_rescheduleShipment {logistics, bus, clock}
    , m_deliverShipment {logistics, bus, clock}
    , m_dispatchNextShipment {logistics, bus, clock}
    , m_listPendingShipments {logistics}
    , m_listShipments {logistics}
    , m_findShipment {logistics}
    , m_getShipment {logistics}
    , m_summarizeZone {logistics}
    , m_findHeaviestShipment {logistics}
    , m_getOperationsMetrics {logistics}
    , m_openSortingBatch {logistics, desk}
    , m_sortSortingBatch {desk}
    , m_prepareSortingBatch {desk}
    , m_processSortingBatchItem {desk}
    , m_returnProcessedItem {desk}
    , m_getActiveSortingBatch {desk}
    , m_getSortingBatch {desk}
    , m_linearSearchInSortingBatch {desk}
    , m_binarySearchInSortingBatch {desk}
    , m_compareSortingBatchSearches {desk}
    , m_assignDeliveryRoute {routing, desk, storage.users, bus, clock}
    , m_resolveRouteStop {logistics, routing, bus, clock}
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
    , signIn {m_signIn}
    , authenticate {m_authenticate}
    , registerUser {m_registerUser}
    , updateUser {m_updateUser}
    , listUsers {m_listUsers}
    , listCouriers {m_listCouriers}
    , findUser {m_findUser}
    , listAuditEvents {m_listAuditEvents}
    , countAuditEvents {m_countAuditEvents}
    , registerShipment {m_registerShipment}
    , classifyShipment {m_classifyShipment}
    , rescheduleShipment {m_rescheduleShipment}
    , deliverShipment {m_deliverShipment}
    , dispatchNextShipment {m_dispatchNextShipment}
    , listPendingShipments {m_listPendingShipments}
    , listShipments {m_listShipments}
    , findShipment {m_findShipment}
    , getShipment {m_getShipment}
    , summarizeZone {m_summarizeZone}
    , findHeaviestShipment {m_findHeaviestShipment}
    , getOperationsMetrics {m_getOperationsMetrics}
    , openSortingBatch {m_openSortingBatch}
    , sortSortingBatch {m_sortSortingBatch}
    , prepareSortingBatch {m_prepareSortingBatch}
    , processSortingBatchItem {m_processSortingBatchItem}
    , returnProcessedItem {m_returnProcessedItem}
    , getActiveSortingBatch {m_getActiveSortingBatch}
    , getSortingBatch {m_getSortingBatch}
    , linearSearchInSortingBatch {m_linearSearchInSortingBatch}
    , binarySearchInSortingBatch {m_binarySearchInSortingBatch}
    , compareSortingBatchSearches {m_compareSortingBatchSearches}
    , assignDeliveryRoute {m_assignDeliveryRoute}
    , resolveRouteStop {m_resolveRouteStop}
    , getRoadMap {m_getRoadMap}
    , previewRoute {m_previewRoute}
    , listRoutes {m_listRoutes}
    , getRoute {m_getRoute}
    , getCourierRouteSheet {m_getCourierRouteSheet}
    , lookupInBst {m_lookupInBst}
    , getBst {m_getBst}
    , lookupInBPlusTree {m_lookupInBPlusTree}
    , rangeInBPlusTree {m_rangeInBPlusTree}
    , getBPlusTree {m_getBPlusTree}
    , compareShipmentStates {m_compareShipmentStates}
    , getCodeSet {m_getCodeSet}
    , combineCodeSets {m_combineCodeSets}
{
    bus.subscribe(m_projector);
    bus.subscribe(m_shipmentPersistence);
    bus.subscribe(m_routePersistence);
    bus.subscribe(m_userPersistence);
    bus.subscribe(m_auditLog);
    logistics::application::LogisticsHydrator {logistics}.hydrate(storage.shipments);
    routing::application::RoutingHydrator {routing}.hydrate(storage.maps, storage.routes);
}

shared::application::Principal ApplicationHarness::admin() const
{
    return shared::application::Principal {1, "Admin", identity::domain::Role::Admin};
}

shared::application::Principal ApplicationHarness::operador() const
{
    return shared::application::Principal {2, "Operador", identity::domain::Role::Operador};
}

shared::application::Principal ApplicationHarness::pum(const std::uint64_t userId) const
{
    return shared::application::Principal {userId, "PUM", identity::domain::Role::Pum};
}

void ApplicationHarness::seedDataset()
{
    for (const auto& draft : initialDataset())
    {
        registerShipment.execute(operador(), draft);
    }
}

identity::domain::UserState ApplicationHarness::bootstrapUser(const identity::application::RegisterUserRequest& request)
{
    return m_registerUser.bootstrap(request, actor());
}

} // namespace hubflow::test
