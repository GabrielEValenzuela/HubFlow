#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_RETURN_PROCESSED_ITEM_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_RETURN_PROCESSED_ITEM_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include <cstdint>
#include <string>

namespace hubflow::sorting::application
{

/**
 * @brief Devuelve a la mesa el último envío procesado, indicado por su código, para corregir una carga equivocada.
 * Si viaja junto con otros, vuelven también los compañeros de su grupo que se cargaron con él.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 * - NotFound: el lote no es el activo o el código no es el último procesado.
 *
 * @par Idempotencia
 * Sí. Sólo prospera si el código indicado es el último procesado; una vez devuelto ya no lo es, y repetir la operación
 * falla con NotFound sin modificar nada.
 */
class ReturnProcessedItemUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~ReturnProcessedItemUseCase() = default;

    virtual ds::DynamicArray<logistics::domain::Shipment*>
    execute(const shared::application::Principal& principal, std::uint64_t batchId, const std::string& code) = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_RETURN_PROCESSED_ITEM_USE_CASE_HPP
