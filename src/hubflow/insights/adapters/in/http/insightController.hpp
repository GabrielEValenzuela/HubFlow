#ifndef HUBFLOW_INSIGHTS_ADAPTERS_IN_HTTP_INSIGHT_CONTROLLER_HPP
#define HUBFLOW_INSIGHTS_ADAPTERS_IN_HTTP_INSIGHT_CONTROLLER_HPP

#include "hubflow/insights/application/ports/in/combineCodeSetsUseCase.hpp"
#include "hubflow/insights/application/ports/in/compareShipmentStatesUseCase.hpp"
#include "hubflow/insights/application/ports/in/getBPlusTreeUseCase.hpp"
#include "hubflow/insights/application/ports/in/getBstUseCase.hpp"
#include "hubflow/insights/application/ports/in/getCodeSetUseCase.hpp"
#include "hubflow/insights/application/ports/in/lookupInBPlusTreeUseCase.hpp"
#include "hubflow/insights/application/ports/in/lookupInBstUseCase.hpp"
#include "hubflow/insights/application/ports/in/rangeInBPlusTreeUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::insights::adapters
{

/// @brief Casos de uso de consulta avanzada que expone el controlador.
struct InsightUseCases
{
    const application::GetBstUseCase&                getBst;
    const application::LookupInBstUseCase&           lookupInBst;
    const application::GetBPlusTreeUseCase&          getBPlusTree;
    const application::LookupInBPlusTreeUseCase&     lookupInBPlusTree;
    const application::RangeInBPlusTreeUseCase&      rangeInBPlusTree;
    const application::CompareShipmentStatesUseCase& compareStates;
    const application::GetCodeSetUseCase&            getCodeSet;
    const application::CombineCodeSetsUseCase&       combineCodeSets;
};

/**
 * @brief Índices de búsqueda y consultas avanzadas (todas son consultas sin efectos).
 *
 * - @c GET /indexes/bst, @c GET /indexes/bst/entries/:code
 * - @c GET /indexes/bplus, @c GET /indexes/bplus/entries/:code, @c GET /indexes/bplus/entries?from=&to=
 * - @c GET /state-history-comparisons?first=&second=
 * - @c GET /shipment-sets?zone=|service=
 * - @c GET /shipment-set-operations?operation=&first=zone:NORTE&second=service:EXPRESS
 */
class InsightController final : public platform::Controller
{
public:
    explicit InsightController(InsightUseCases useCases);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    platform::HttpResponse bPlusEntry(const platform::RequestContext& context) const;
    platform::HttpResponse bPlusRange(const platform::RequestContext& context) const;
    platform::HttpResponse stateComparison(const platform::RequestContext& context) const;
    platform::HttpResponse shipmentSet(const platform::RequestContext& context) const;
    platform::HttpResponse setOperation(const platform::RequestContext& context) const;

    InsightUseCases m_useCases;
};

} // namespace hubflow::insights::adapters

#endif // HUBFLOW_INSIGHTS_ADAPTERS_IN_HTTP_INSIGHT_CONTROLLER_HPP
