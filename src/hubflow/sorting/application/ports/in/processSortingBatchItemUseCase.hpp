#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_PROCESS_SORTING_BATCH_ITEM_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_PROCESS_SORTING_BATCH_ITEM_USE_CASE_HPP

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
 * @brief Carga envíos al vehículo: los indicados por código o, si no se indica ninguno, el siguiente de la cola de
 * preparación. Los envíos se cargan completos, con todos sus bultos.
 *
 * Un envío que viaja junto con otros se carga siempre con ellos. Si no se indican códigos y el siguiente de la cola
 * pertenece a un grupo, se cargan todos los del grupo. Si se indican códigos y falta algún compañero de grupo, se
 * rechaza todo sin cargar nada.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 * - NotFound: el lote no es el activo o un código no está en la lista por cargar.
 * - Conflict: la cola de preparación está vacía o el grupo quedaría incompleto.
 *
 * @par Idempotencia
 * No. Cada ejecución carga envíos distintos: repetirla avanza la cola. Con Idempotency-Key el borde HTTP evita
 * cargar dos veces por un reintento.
 */
class ProcessSortingBatchItemUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~ProcessSortingBatchItemUseCase() = default;

    virtual ds::DynamicArray<logistics::domain::Shipment*> execute(const shared::application::Principal& principal,
                                                                   std::uint64_t                         batchId,
                                                                   const ds::DynamicArray<std::string>&  codes) = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_PROCESS_SORTING_BATCH_ITEM_USE_CASE_HPP
