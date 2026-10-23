#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_OPEN_SORTING_BATCH_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_OPEN_SORTING_BATCH_USE_CASE_HPP

#include "hubflow/logistics/domain/valueobjects/zone.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::sorting::application
{

/**
 * @brief Abre un lote de la mesa de clasificación con los envíos pendientes de una zona, reemplazando al lote anterior.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 * - Conflict: la zona no tiene envíos pendientes.
 *
 * @par Idempotencia
 * No. Cada ejecución crea un lote nuevo con su propio identificador y descarta el anterior. Con Idempotency-Key el
 * borde HTTP evita abrir dos lotes por un reintento.
 */
class OpenSortingBatchUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~OpenSortingBatchUseCase() = default;

    virtual const domain::SortingBatch& execute(const shared::application::Principal& principal,
                                                logistics::domain::Zone               zone) = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_OPEN_SORTING_BATCH_USE_CASE_HPP
