#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_GET_ACTIVE_SORTING_BATCH_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_GET_ACTIVE_SORTING_BATCH_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/**
 * @brief Devuelve el lote que está abierto en la mesa, o ninguno (nullptr) si no hay.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class GetActiveSortingBatchUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~GetActiveSortingBatchUseCase() = default;

    virtual const domain::SortingBatch* execute(const shared::application::Principal& principal) const = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_GET_ACTIVE_SORTING_BATCH_USE_CASE_HPP
