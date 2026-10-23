#ifndef HUBFLOW_SORTING_ADAPTERS_IN_HTTP_SORTING_JSON_PRESENTER_HPP
#define HUBFLOW_SORTING_ADAPTERS_IN_HTTP_SORTING_JSON_PRESENTER_HPP

#include "hubflow/platform/http/httpTypes.hpp"
#include "hubflow/sorting/domain/entities/sortingBatch.hpp"
#include "hubflow/sorting/domain/services/batchSearch.hpp"
#include "hubflow/sorting/domain/services/insertionSort.hpp"

namespace hubflow::sorting::adapters
{

/// @brief Representación externa de la mesa de clasificación.
class SortingJsonPresenter final
{
public:
    /// @brief Lote con sus envíos, cola de preparación y pila de procesados.
    static platform::Json sortingBatch(const domain::SortingBatch& batch);

    /// @brief Comparaciones y desplazamientos que hizo el ordenamiento.
    static platform::Json sortStats(const domain::SortStats& stats);

    /// @brief Resultado de una búsqueda dentro del lote, con los pasos de la búsqueda binaria si los hubo.
    static platform::Json batchSearch(const domain::BatchSearchOutcome& outcome, const domain::SortingBatch& batch);
};

} // namespace hubflow::sorting::adapters

#endif // HUBFLOW_SORTING_ADAPTERS_IN_HTTP_SORTING_JSON_PRESENTER_HPP
