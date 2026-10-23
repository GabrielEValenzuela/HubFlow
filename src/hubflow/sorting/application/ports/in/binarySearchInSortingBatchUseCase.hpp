#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_BINARY_SEARCH_IN_SORTING_BATCH_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_BINARY_SEARCH_IN_SORTING_BATCH_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/batchSearch.hpp"

#include <cstdint>
#include <string>

namespace hubflow::sorting::application
{

/**
 * @brief Busca un envío del lote por código dividiendo el rango a la mitad en cada paso; exige que el lote esté
 * ordenado por código.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 * - NotFound: el lote no es el activo.
 * - Conflict: el lote no está ordenado por código.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class BinarySearchInSortingBatchUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~BinarySearchInSortingBatchUseCase() = default;

    virtual domain::BatchSearchOutcome
    execute(const shared::application::Principal& principal, std::uint64_t batchId, const std::string& code) const = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_BINARY_SEARCH_IN_SORTING_BATCH_USE_CASE_HPP
