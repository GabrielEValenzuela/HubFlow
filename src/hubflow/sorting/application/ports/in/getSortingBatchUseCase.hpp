#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_GET_SORTING_BATCH_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_GET_SORTING_BATCH_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include <cstdint>

namespace hubflow::sorting::application
{

    /**
     * @brief Devuelve un lote de la mesa por su identificador.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar la mesa de clasificación.
     * - NotFound: el lote no es el activo.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class GetSortingBatchUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetSortingBatchUseCase() = default;

        virtual const domain::SortingBatch& execute(const shared::application::Principal& principal,
                                                    std::uint64_t batchId) const = 0;
    };

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_GET_SORTING_BATCH_USE_CASE_HPP
