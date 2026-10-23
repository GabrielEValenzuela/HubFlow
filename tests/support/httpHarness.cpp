#include "support/httpHarness.hpp"

#include "hubflow/audit/adapters/in/http/auditController.hpp"
#include "hubflow/identity/adapters/in/http/sessionController.hpp"
#include "hubflow/identity/adapters/in/http/userController.hpp"
#include "hubflow/insights/adapters/in/http/insightController.hpp"
#include "hubflow/logistics/adapters/in/http/catalogController.hpp"
#include "hubflow/logistics/adapters/in/http/dispatchController.hpp"
#include "hubflow/logistics/adapters/in/http/movementController.hpp"
#include "hubflow/logistics/adapters/in/http/operationsController.hpp"
#include "hubflow/logistics/adapters/in/http/shipmentController.hpp"
#include "hubflow/logistics/adapters/in/http/shipmentGroupController.hpp"
#include "hubflow/platform/http/healthController.hpp"
#include "hubflow/routing/adapters/in/http/roadMapController.hpp"
#include "hubflow/routing/adapters/in/http/routeController.hpp"
#include "hubflow/sorting/adapters/in/http/sortingBatchController.hpp"

namespace hubflow::test
{
namespace
{
using identity::application::RegisterUserRequest;
using identity::domain::Role;
} // namespace

HttpHarness::HttpHarness()
    : app {storage}
{
    app.bootstrapUser(RegisterUserRequest {"admin@hubflow.local", "Admin", Role::Admin, "clave-segura"});
    operadorId =
        app.bootstrapUser(RegisterUserRequest {"operador@hubflow.local", "Operador", Role::Operador, "clave-segura"})
            .id;
    pumId      = app.bootstrapUser(RegisterUserRequest {"pum1@hubflow.local", "Pum Uno", Role::Pum, "clave-segura"}).id;
    otherPumId = app.bootstrapUser(RegisterUserRequest {"pum2@hubflow.local", "Pum Dos", Role::Pum, "clave-segura"}).id;

    m_spRouter = std::make_unique<platform::ApiRouter>(m_server, app.authenticate, idempotency, m_mutex);
    m_controllers.push_back(std::make_unique<platform::HealthController>(
        []
        {
            return true;
        }));
    m_controllers.push_back(std::make_unique<identity::adapters::SessionController>(app.signIn, app.findUser));
    m_controllers.push_back(std::make_unique<identity::adapters::UserController>(identity::adapters::UserUseCases {
        app.registerUser, app.updateUser, app.listUsers, app.listCouriers, app.findUser}));
    m_controllers.push_back(
        std::make_unique<audit::adapters::AuditController>(app.listAuditEvents, app.countAuditEvents));
    m_controllers.push_back(std::make_unique<logistics::adapters::ShipmentController>(
        logistics::adapters::ShipmentUseCases {app.registerShipment,
                                               app.listShipments,
                                               app.findShipment,
                                               app.getShipment,
                                               app.findShipmentGroupOfShipment}));
    m_controllers.push_back(
        std::make_unique<logistics::adapters::ShipmentGroupController>(logistics::adapters::ShipmentGroupUseCases {
            app.formShipmentGroup, app.dissolveShipmentGroup, app.listShipmentGroups, app.getShipmentGroup}));
    m_controllers.push_back(
        std::make_unique<logistics::adapters::MovementController>(logistics::adapters::MovementUseCases {
            app.classifyShipment, app.rescheduleShipment, app.deliverShipment, app.getShipment}));
    m_controllers.push_back(
        std::make_unique<logistics::adapters::DispatchController>(app.dispatchNextShipment, app.listPendingShipments));
    m_controllers.push_back(
        std::make_unique<logistics::adapters::OperationsController>(logistics::adapters::OperationsUseCases {
            app.summarizeZone, app.findHeaviestShipment, app.getOperationsMetrics}));
    m_controllers.push_back(std::make_unique<logistics::adapters::CatalogController>(
        logistics::adapters::CatalogUseCases {app.createZone,
                                              app.updateZone,
                                              app.listZones,
                                              app.createServiceLevel,
                                              app.updateServiceLevel,
                                              app.listServiceLevels,
                                              app.createShipmentFeature,
                                              app.updateShipmentFeature,
                                              app.listShipmentFeatures}));
    m_controllers.push_back(std::make_unique<sorting::adapters::SortingBatchController>(
        sorting::adapters::SortingUseCases {app.openSortingBatch,
                                            app.sortSortingBatch,
                                            app.prepareSortingBatch,
                                            app.processSortingBatchItem,
                                            app.returnProcessedItem,
                                            app.getActiveSortingBatch,
                                            app.getSortingBatch,
                                            app.linearSearchInSortingBatch,
                                            app.binarySearchInSortingBatch,
                                            app.compareSortingBatchSearches}));
    m_controllers.push_back(std::make_unique<routing::adapters::RouteController>(
        routing::adapters::RouteUseCases {app.assignDeliveryRoute,
                                          app.resolveRouteStop,
                                          app.confirmStopParcel,
                                          app.reassignDeliveryRoute,
                                          app.listRoutes,
                                          app.getRoute,
                                          app.getRoadMap,
                                          app.getCourierRouteSheet}));
    m_controllers.push_back(std::make_unique<routing::adapters::RoadMapController>(
        app.getRoadMap, app.previewRoute, app.addRoadNode, app.updateRoadNode));
    m_controllers.push_back(std::make_unique<insights::adapters::InsightController>(
        insights::adapters::InsightUseCases {app.getBst,
                                             app.lookupInBst,
                                             app.getBPlusTree,
                                             app.lookupInBPlusTree,
                                             app.rangeInBPlusTree,
                                             app.compareShipmentStates,
                                             app.getCodeSet,
                                             app.combineCodeSets}));
    for (auto& spController : m_controllers)
    {
        spController->registerRoutes(*m_spRouter);
    }
    const int port = m_server.bind_to_any_port("127.0.0.1");
    m_thread       = std::thread {[this]
                            {
                                m_server.listen_after_bind();
                            }};
    m_server.wait_until_ready();
    m_spClient = std::make_unique<httplib::Client>("127.0.0.1", port);
}

HttpHarness::~HttpHarness()
{
    m_server.stop();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

httplib::Client& HttpHarness::client()
{
    return *m_spClient;
}

std::string HttpHarness::login(const std::string& email, const std::string& password)
{
    const nlohmann::json body {{"email", email}, {"password", password}};
    const auto           result = m_spClient->Post("/api/v1/sessions", body.dump(), "application/json");
    return bodyOf(result).at("token").get<std::string>();
}

httplib::Headers HttpHarness::as(const std::string& email, httplib::Headers extra)
{
    extra.emplace("Authorization", "Bearer " + login(email));
    return extra;
}

nlohmann::json HttpHarness::bodyOf(const httplib::Result& result)
{
    return nlohmann::json::parse(result->body, nullptr, false);
}

} // namespace hubflow::test
