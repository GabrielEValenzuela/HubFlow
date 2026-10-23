#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_SORTING_CONTROLLER_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_SORTING_CONTROLLER_HPP

#include "application/sorting/sortingDeskCommands.hpp"
#include "application/sorting/sortingQueries.hpp"
#include "infrastructure/http/controllers/controller.hpp"

namespace hubflow::infrastructure
{

/// @brief Casos de uso de la mesa de clasificación.
struct SortingUseCases
{
    application::OpenSortingBatch&        open;
    application::SortSortingBatch&        sort;
    application::PrepareSortingBatch&     prepare;
    application::ProcessSortingBatchItem& process;
    application::UndoSortingBatchItem&    undo;
    const application::SortingQueries&    queries;
};

/**
 * @brief Mesa de clasificación y despacho (RF09).
 *
 * - @c POST /sorting-batches {zone} — RF09.1
 * - @c GET /sorting-batches/current, @c GET /sorting-batches/:id
 * - @c PATCH /sorting-batches/:id {sortBy} — RF09.2
 * - @c PUT /sorting-batches/:id/preparation-queue — RF09.3
 * - @c POST /sorting-batches/:id/processed-items — RF09.4 (dequeue + push)
 * - @c DELETE /sorting-batches/:id/processed-items/top — pop
 * - @c GET /sorting-batches/:id/searches?code=&algorithm=linear|binary|both — RF09.5–RF09.7
 */
class SortingController final : public Controller
{
public:
    explicit SortingController(SortingUseCases useCases);

private:
    void registerRoutes(ApiRouter& router) override;

    HttpResponse search(const RequestContext& context) const;

    SortingUseCases m_useCases;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_SORTING_CONTROLLER_HPP
