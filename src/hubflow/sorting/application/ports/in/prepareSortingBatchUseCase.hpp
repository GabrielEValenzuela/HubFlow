#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_PREPARE_SORTING_BATCH_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_PREPARE_SORTING_BATCH_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include <cstdint>

namespace hubflow::sorting::application
{

/**
 * @brief Carga el lote, en su orden actual, en la cola de preparación desde la que se irán procesando los envíos.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 * - NotFound: el lote no es el activo.
 *
 * @par Idempotencia
 * Sí. La cola se reconstruye desde el lote en cada ejecución (la anterior se descarta), por lo que repetirla deja el
 * mismo estado.
 */
class PrepareSortingBatchUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~PrepareSortingBatchUseCase() = default;

    virtual const domain::SortingBatch& execute(const shared::application::Principal& principal,
                                                std::uint64_t                         batchId) = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_PREPARE_SORTING_BATCH_USE_CASE_HPP
