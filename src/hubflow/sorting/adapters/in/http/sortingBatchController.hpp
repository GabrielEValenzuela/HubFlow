#ifndef HUBFLOW_SORTING_ADAPTERS_IN_HTTP_SORTING_BATCH_CONTROLLER_HPP
#define HUBFLOW_SORTING_ADAPTERS_IN_HTTP_SORTING_BATCH_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"
#include "hubflow/sorting/application/ports/in/binarySearchInSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/compareSortingBatchSearchesUseCase.hpp"
#include "hubflow/sorting/application/ports/in/getActiveSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/getSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/linearSearchInSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/openSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/prepareSortingBatchUseCase.hpp"
#include "hubflow/sorting/application/ports/in/processSortingBatchItemUseCase.hpp"
#include "hubflow/sorting/application/ports/in/returnProcessedItemUseCase.hpp"
#include "hubflow/sorting/application/ports/in/sortSortingBatchUseCase.hpp"

namespace hubflow::sorting::adapters
{

/// @brief Casos de uso de la mesa de clasificación que expone el controlador.
struct SortingUseCases
{
    application::OpenSortingBatchUseCase&                  open;
    application::SortSortingBatchUseCase&                  sort;
    application::PrepareSortingBatchUseCase&               prepare;
    application::ProcessSortingBatchItemUseCase&           process;
    application::ReturnProcessedItemUseCase&               giveBack;
    const application::GetActiveSortingBatchUseCase&       active;
    const application::GetSortingBatchUseCase&             batch;
    const application::LinearSearchInSortingBatchUseCase&  linearSearch;
    const application::BinarySearchInSortingBatchUseCase&  binarySearch;
    const application::CompareSortingBatchSearchesUseCase& compare;
};

/**
 * @brief Mesa de clasificación y despacho de un lote por zona.
 *
 * - @c POST /sorting-batches {zone}: abre un lote con los envíos pendientes de la zona (acepta @c Idempotency-Key).
 * - @c GET /sorting-batches/current, @c GET /sorting-batches/:id: consulta del lote.
 * - @c PATCH /sorting-batches/:id {sortBy}: ordena el lote; mismo criterio, mismo orden.
 * - @c PUT /sorting-batches/:id/preparation-queue: recarga la cola de preparación desde el lote.
 * - @c POST /sorting-batches/:id/processed-items: procesa el siguiente envío de la cola (acepta @c Idempotency-Key).
 * - @c DELETE /sorting-batches/:id/processed-items/:code: devuelve el último procesado; sólo si @c code es el
 *   tope de la pila, y repetirlo responde 404 sin efecto.
 * - @c GET /sorting-batches/:id/searches?code=&algorithm=linear|binary|both: busca un envío en el lote.
 */
class SortingBatchController final : public platform::Controller
{
public:
    explicit SortingBatchController(SortingUseCases useCases);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    platform::HttpResponse sort(const platform::RequestContext& context) const;
    platform::HttpResponse search(const platform::RequestContext& context) const;
    platform::HttpResponse processNext(const platform::RequestContext& context) const;
    platform::HttpResponse giveBack(const platform::RequestContext& context) const;

    SortingUseCases m_useCases;
};

} // namespace hubflow::sorting::adapters

#endif // HUBFLOW_SORTING_ADAPTERS_IN_HTTP_SORTING_BATCH_CONTROLLER_HPP
