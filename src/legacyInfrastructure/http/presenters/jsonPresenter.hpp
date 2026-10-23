#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_JSON_PRESENTER_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_JSON_PRESENTER_HPP

#include "application/audit/auditTrail.hpp"
#include "application/identity/sessionCommands.hpp"
#include "application/insights/insightQueries.hpp"
#include "application/logistics/shipmentQueries.hpp"
#include "application/sorting/sortingQueries.hpp"
#include "domain/routing/aggregates/deliveryRoute.hpp"
#include "infrastructure/http/httpTypes.hpp"

namespace hubflow::infrastructure
{

/**
 * @brief Traduce el modelo de dominio y los resultados de los casos de uso a JSON.
 *
 * Es la única pieza que conoce la representación externa; el dominio no sabe de JSON.
 */
class JsonPresenter final
{
public:
    static Json user(const domain::identity::UserState& user);
    static Json session(const application::Session& session);

    static Json shipment(const domain::logistics::Shipment& shipment);
    static Json shipmentSummary(const domain::logistics::Shipment& shipment);
    static Json movement(const domain::logistics::Movement& movement);
    static Json history(const domain::logistics::Shipment& shipment, bool newestFirst);
    static Json stateHistory(const domain::logistics::StateHistory& bits);
    static Json zoneSummary(domain::logistics::Zone zone, const domain::logistics::ZoneSummary& summary);
    static Json metrics(const application::OperationsMetrics& metrics);

    static Json sortingBatch(const domain::sorting::SortingBatch& batch);
    static Json sortStats(const domain::sorting::SortStats& stats);
    static Json batchSearch(const domain::sorting::BatchSearchOutcome& outcome, const domain::sorting::SortingBatch& batch);

    static Json bst(const domain::logistics::ShipmentBst& tree);
    static Json bstLookup(const application::BstLookup& lookup);
    static Json bPlusTree(const domain::logistics::ShipmentBPlusTree& tree);
    static Json codeSet(const domain::insights::CodeSet& set);
    static Json stateComparison(const domain::insights::StateComparison& comparison);
    static Json shipments(const ds::DynamicArray<const domain::logistics::Shipment*>& shipments);

    static Json roadMap(const domain::routing::RoadMap& map);
    static Json searchTrace(const domain::routing::SearchTrace& trace, const domain::routing::RoadMap& map);
    static Json routeSummary(const domain::routing::DeliveryRoute& route);
    static Json route(const domain::routing::DeliveryRoute& route, const domain::routing::RoadMap& map);

    /// @brief Export equivalente a @c initial_state.json del simulador de Hanoi.
    static Json initialState(const domain::routing::DeliveryRoute& route, const domain::routing::RoadMap& map);

    /// @brief Export equivalente a @c sequence.json del simulador de Hanoi.
    static Json sequence(const domain::routing::DeliveryRoute& route, const domain::routing::RoadMap& map);

    static Json auditEntry(const application::AuditEntry& entry);
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_JSON_PRESENTER_HPP
