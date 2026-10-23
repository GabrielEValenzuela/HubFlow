#ifndef HUBFLOW_SORTING_APPLICATION_PORTS_IN_PROCESS_SORTING_BATCH_ITEM_USE_CASE_HPP
#define HUBFLOW_SORTING_APPLICATION_PORTS_IN_PROCESS_SORTING_BATCH_ITEM_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include <cstdint>

namespace hubflow::sorting::application
{

/**
 * @brief Saca el siguiente envío de la cola de preparación y lo apila como procesado (cargado al vehículo).
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar la mesa de clasificación.
 * - NotFound: el lote no es el activo.
 * - Conflict: la cola de preparación está vacía.
 *
 * @par Idempotencia
 * No. Cada ejecución procesa un envío distinto: repetirla avanza la cola. Con Idempotency-Key el borde HTTP evita
 * procesar dos por un reintento.
 */
class ProcessSortingBatchItemUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~ProcessSortingBatchItemUseCase() = default;

    virtual const logistics::domain::Shipment& execute(const shared::application::Principal& principal,
                                                       std::uint64_t                         batchId) = 0;
};

} // namespace hubflow::sorting::application

#endif // HUBFLOW_SORTING_APPLICATION_PORTS_IN_PROCESS_SORTING_BATCH_ITEM_USE_CASE_HPP
