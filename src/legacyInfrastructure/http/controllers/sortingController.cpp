#include "infrastructure/http/controllers/sortingController.hpp"

#include "application/shared/vocabulary.hpp"
#include "domain/shared/businessError.hpp"
#include "infrastructure/http/presenters/jsonPresenter.hpp"
#include "infrastructure/http/validation/inputValidator.hpp"

namespace hubflow::infrastructure
{
namespace
{
using application::Vocabulary;

constexpr const char* ZONES {"CENTRO, NORTE, SUR"};
constexpr const char* CRITERIA {"CODE_ASC, WEIGHT_DESC"};
constexpr const char* ALGORITHMS {"linear, binary, both"};

enum class SearchMode
{
    Linear,
    Binary,
    Both
};

std::optional<SearchMode> searchMode(const std::string& text)
{
    std::optional<SearchMode> mode;
    if (text == "linear")
    {
        mode = SearchMode::Linear;
    }
    else if (text == "binary")
    {
        mode = SearchMode::Binary;
    }
    else if (text == "both")
    {
        mode = SearchMode::Both;
    }
    return mode;
}

std::optional<domain::sorting::SortCriteria> sortableCriteria(const std::string& text)
{
    const auto criteria = Vocabulary::sortCriteria(text);
    return criteria == domain::sorting::SortCriteria::None ? std::nullopt : criteria;
}

std::uint64_t batchId(const RequestContext& context)
{
    auto                input = InputValidator::parameters(context);
    const std::uint64_t id {input.requiredIdentifier("id")};
    input.validate();
    return id;
}

Json withShipment(const domain::sorting::SortingBatch& batch, const domain::logistics::Shipment& shipment)
{
    return Json {{"shipment", JsonPresenter::shipmentSummary(shipment)}, {"batch", JsonPresenter::sortingBatch(batch)}};
}
} // namespace

SortingController::SortingController(SortingUseCases useCases)
    : m_useCases {useCases}
{
}

void SortingController::registerRoutes(ApiRouter& router)
{
    router.post("/sorting-batches",
                [this](const RequestContext& context)
                {
                    auto       input = InputValidator::body(context.request);
                    const auto zone  = input.requiredTerm<domain::logistics::Zone>("zone", Vocabulary::zone, ZONES);
                    input.validate();
                    const auto& batch = m_useCases.open.execute(context.user(), zone);
                    return HttpResponse::created(JsonPresenter::sortingBatch(batch),
                                                 "/api/v1/sorting-batches/" + std::to_string(batch.id()));
                });
    router.get("/sorting-batches/current",
               [this](const RequestContext& context)
               {
                   const auto* pBatch = m_useCases.queries.active(context.user());
                   if (pBatch == nullptr)
                   {
                       throw domain::business_error {domain::ErrorKind::NotFound, "sorting.no_batch", "No hay un lote abierto"};
                   }
                   return HttpResponse::ok(JsonPresenter::sortingBatch(*pBatch));
               });
    router.get("/sorting-batches/:id",
               [this](const RequestContext& context)
               { return HttpResponse::ok(JsonPresenter::sortingBatch(m_useCases.queries.batch(context.user(), batchId(context)))); });
    router.patch("/sorting-batches/:id",
                 [this](const RequestContext& context)
                 {
                     const std::uint64_t id {batchId(context)};
                     auto                input = InputValidator::body(context.request);
                     const auto criteria = input.requiredTerm<domain::sorting::SortCriteria>("sortBy", sortableCriteria, CRITERIA);
                     input.validate();
                     const auto stats = m_useCases.sort.execute(context.user(), id, criteria);
                     return HttpResponse::ok(Json {{"batch", JsonPresenter::sortingBatch(m_useCases.queries.batch(context.user(), id))},
                                                   {"stats", JsonPresenter::sortStats(stats)}});
                 });
    router.put("/sorting-batches/:id/preparation-queue",
               [this](const RequestContext& context)
               { return HttpResponse::ok(JsonPresenter::sortingBatch(m_useCases.prepare.execute(context.user(), batchId(context)))); });
    router.post("/sorting-batches/:id/processed-items",
                [this](const RequestContext& context)
                {
                    const std::uint64_t id {batchId(context)};
                    const auto&         shipment = m_useCases.process.execute(context.user(), id);
                    return HttpResponse::created(withShipment(m_useCases.queries.batch(context.user(), id), shipment),
                                                 "/api/v1/sorting-batches/" + std::to_string(id) + "/processed-items/top");
                });
    router.remove("/sorting-batches/:id/processed-items/top",
                  [this](const RequestContext& context)
                  {
                      const std::uint64_t id {batchId(context)};
                      const auto&         shipment = m_useCases.undo.execute(context.user(), id);
                      return HttpResponse::ok(withShipment(m_useCases.queries.batch(context.user(), id), shipment));
                  });
    router.get("/sorting-batches/:id/searches", [this](const RequestContext& context) { return search(context); });
}

HttpResponse SortingController::search(const RequestContext& context) const
{
    auto                input = InputValidator::parameters(context);
    const std::uint64_t id {input.requiredIdentifier("id")};
    const std::string   code {input.trackingCode("code")};
    const SearchMode    mode {input.optionalTerm<SearchMode>("algorithm", searchMode, ALGORITHMS).value_or(SearchMode::Both)};
    input.validate();
    const auto& batch = m_useCases.queries.batch(context.user(), id);
    Json        body {{"code", code}, {"batchSortedBy", Vocabulary::name(batch.sortedBy())}};
    if (mode == SearchMode::Linear)
    {
        body["linear"] = JsonPresenter::batchSearch(m_useCases.queries.linearSearch(context.user(), id, code), batch);
    }
    else if (mode == SearchMode::Binary)
    {
        body["binary"] = JsonPresenter::batchSearch(m_useCases.queries.binarySearch(context.user(), id, code), batch);
    }
    else
    {
        const auto comparison = m_useCases.queries.compare(context.user(), id, code);
        body["linear"]        = JsonPresenter::batchSearch(comparison.linear, batch);
        body["binary"] = comparison.binary.has_value() ? JsonPresenter::batchSearch(*comparison.binary, batch) : Json(nullptr);
        body["binaryUnavailableReason"] = comparison.binaryUnavailableReason;
    }
    return HttpResponse::ok(body);
}

} // namespace hubflow::infrastructure
