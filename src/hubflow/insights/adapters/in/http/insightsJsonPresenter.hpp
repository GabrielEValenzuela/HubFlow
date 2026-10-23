#ifndef HUBFLOW_INSIGHTS_ADAPTERS_IN_HTTP_INSIGHTS_JSON_PRESENTER_HPP
#define HUBFLOW_INSIGHTS_ADAPTERS_IN_HTTP_INSIGHTS_JSON_PRESENTER_HPP

#include "hubflow/insights/application/ports/in/getCodeSetUseCase.hpp"
#include "hubflow/insights/application/ports/in/lookupInBstUseCase.hpp"
#include "hubflow/insights/domain/services/codeSet.hpp"
#include "hubflow/insights/domain/services/stateComparison.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/platform/http/httpTypes.hpp"

namespace hubflow::insights::adapters
{

/// @brief Representación externa de los índices de búsqueda y las consultas por conjuntos.
class InsightsJsonPresenter final
{
public:
    /// @brief Árbol binario de búsqueda: tamaño, altura, recorrido en orden y estructura.
    static platform::Json bst(const logistics::domain::ShipmentBst& tree);

    /// @brief Búsqueda en el árbol binario con el camino recorrido.
    static platform::Json bstLookup(const application::BstLookup& lookup);

    /// @brief Árbol B+: niveles, secuencia de hojas y estructura.
    static platform::Json bPlusTree(const logistics::domain::ShipmentBPlusTree& tree);

    /// @brief Conjunto de códigos de envío.
    static platform::Json codeSet(const domain::CodeSet& set);

    /// @brief Comparación bit a bit de dos historiales de estados.
    static platform::Json stateComparison(const domain::StateComparison& comparison);

    /// @brief Criterio con el que se armó un conjunto: zona o nivel de servicio.
    static platform::Json selector(const application::SetSelector& selector);
};

} // namespace hubflow::insights::adapters

#endif // HUBFLOW_INSIGHTS_ADAPTERS_IN_HTTP_INSIGHTS_JSON_PRESENTER_HPP
