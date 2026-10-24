#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_SORT_SORTING_BATCH_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_SORT_SORTING_BATCH_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include <cstdint>

namespace hubflow::sorting::application
{

    /**
     * @brief Ordena el lote con inserción según un criterio (código o peso) y devuelve cuántas comparaciones y
     * desplazamientos hizo.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar la mesa de clasificación.
     * - NotFound: el lote no es el activo.
     *
     * @par Idempotencia
     * Sí. Ordenar con el mismo criterio siempre deja el lote en el mismo orden; repetirlo no cambia el resultado.
     */
    class SortSortingBatchUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~SortSortingBatchUseCase() = default;

        virtual domain::SortStats execute(const shared::application::Principal& principal,
                                          std::uint64_t batchId,
                                          domain::SortCriteria criteria) = 0;
    };

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_SORT_SORTING_BATCH_USE_CASE_HPP
