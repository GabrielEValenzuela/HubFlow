#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_COMPARE_SORTING_BATCH_SEARCHES_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_COMPARE_SORTING_BATCH_SEARCHES_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/batchSearch.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace hubflow::sorting::application
{

/// @brief Resultado de buscar el mismo código con ambos algoritmos.
struct SearchComparison
{
    domain::BatchSearchOutcome                linear;
    std::optional<domain::BatchSearchOutcome> binary;
    std::string                               binaryUnavailableReason;
};

/**
 * @brief Busca el mismo código con ambos algoritmos sobre el mismo lote para comparar su costo; la búsqueda binaria
 * sólo corre si el lote está ordenado por código.
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
class CompareSortingBatchSearchesUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~CompareSortingBatchSearchesUseCase() = default;

    virtual SearchComparison
    execute(const shared::application::Principal& principal, std::uint64_t batchId, const std::string& code) const = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_COMPARE_SORTING_BATCH_SEARCHES_USE_CASE_HPP
