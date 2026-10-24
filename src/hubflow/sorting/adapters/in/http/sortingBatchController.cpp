#include "hubflow/sorting/adapters/in/http/sortingBatchController.hpp"
#include <cstdint>

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/shared/application/vocabulary.hpp"
#include "hubflow/shared/domain/businessError.hpp"
#include "hubflow/sorting/adapters/in/http/sortingJsonPresenter.hpp"

namespace hubflow::sorting::adapters
{
    namespace
    {
        using logistics::adapters::LogisticsJsonPresenter;
        using platform::InputValidator;
        using platform::Json;
        using platform::RequestContext;
        using shared::application::Vocabulary;

        constexpr const char* CRITERIA {"CODE_ASC, WEIGHT_DESC"};
        constexpr const char* ALGORITHMS {"linear, binary, both"};
        constexpr std::size_t MAX_CODES {100};

        Json shipmentList(const ds::DynamicArray<logistics::domain::Shipment*>& shipments)
        {
            Json items = Json::array();
            for (std::size_t shipmentIndex = 0; shipmentIndex < shipments.size(); ++shipmentIndex)
            {
                items.push_back(LogisticsJsonPresenter::shipmentSummary(*shipments[shipmentIndex]));
            }
            return items;
        }

        enum class SearchMode : std::uint8_t
        {
            Linear,
            Binary,
            Both
        };

        struct SearchModeName
        {
            const char* text;
            SearchMode mode;
        };

        constexpr SearchModeName SEARCH_MODES[] {
            {"linear", SearchMode::Linear}, {"binary", SearchMode::Binary}, {"both", SearchMode::Both}};

        std::optional<SearchMode> searchMode(const std::string& text)
        {
            std::optional<SearchMode> found;
            for (const auto& entry : SEARCH_MODES)
            {
                if (text == entry.text)
                {
                    found = entry.mode;
                }
            }
            return found;
        }

        std::optional<domain::SortCriteria> sortableCriteria(const std::string& text)
        {
            const auto criteria = Vocabulary::sortCriteria(text);
            return criteria == domain::SortCriteria::None ? std::nullopt : criteria;
        }

        std::uint64_t batchId(const RequestContext& context)
        {
            auto input = InputValidator::parameters(context);
            const std::uint64_t id {input.requiredIdentifier("id")};
            input.validate();
            return id;
        }
    } // namespace

    SortingBatchController::SortingBatchController(const SortingUseCases& useCases)
        : m_useCases {useCases}
    {
    }

    void SortingBatchController::registerRoutes(platform::ApiRouter& router)
    {
        router.post(
            "/sorting-batches",
            [this](const RequestContext& context)
            {
                auto input = InputValidator::body(context.request);
                const auto zone = input.code("zone");
                input.validate();
                const auto& batch = m_useCases.open.execute(context.user(), zone);
                return platform::HttpResponse::created(SortingJsonPresenter::sortingBatch(batch),
                                                       locationOf("/sorting-batches/" + std::to_string(batch.id())));
            },
            application::OpenSortingBatchUseCase::IDEMPOTENCY);
        router.get("/sorting-batches/current",
                   [this](const RequestContext& context)
                   {
                       const auto* pBatch = m_useCases.active.execute(context.user());
                       if (pBatch == nullptr)
                       {
                           throw shared::domain::BusinessError {
                               shared::domain::ErrorKind::NotFound, "sorting.no_batch", "No hay un lote abierto"};
                       }
                       return platform::HttpResponse::ok(SortingJsonPresenter::sortingBatch(*pBatch));
                   });
        router.get("/sorting-batches/:id",
                   [this](const RequestContext& context)
                   {
                       return platform::HttpResponse::ok(SortingJsonPresenter::sortingBatch(
                           m_useCases.batch.execute(context.user(), batchId(context))));
                   });
        router.patch("/sorting-batches/:id", [this](const RequestContext& context) { return sort(context); });
        router.put("/sorting-batches/:id/preparation-queue",
                   [this](const RequestContext& context)
                   {
                       return platform::HttpResponse::ok(SortingJsonPresenter::sortingBatch(
                           m_useCases.prepare.execute(context.user(), batchId(context))));
                   });
        router.post(
            "/sorting-batches/:id/processed-items",
            [this](const RequestContext& context) { return processNext(context); },
            application::ProcessSortingBatchItemUseCase::IDEMPOTENCY);
        router.remove("/sorting-batches/:id/processed-items/:code",
                      [this](const RequestContext& context) { return giveBack(context); });
        router.get("/sorting-batches/:id/searches", [this](const RequestContext& context) { return search(context); });
    }

    platform::HttpResponse SortingBatchController::sort(const RequestContext& context) const
    {
        const std::uint64_t id {batchId(context)};
        auto input = InputValidator::body(context.request);
        const auto criteria = input.requiredTerm<domain::SortCriteria>("sortBy", sortableCriteria, CRITERIA);
        input.validate();
        const auto stats = m_useCases.sort.execute(context.user(), id, criteria);
        return platform::HttpResponse::ok(
            Json {{"batch", SortingJsonPresenter::sortingBatch(m_useCases.batch.execute(context.user(), id))},
                  {"stats", SortingJsonPresenter::sortStats(stats)}});
    }

    platform::HttpResponse SortingBatchController::processNext(const RequestContext& context) const
    {
        const std::uint64_t id {batchId(context)};
        auto input = InputValidator::body(context.request);
        ds::DynamicArray<std::string> codes;
        if (input.has("codes"))
        {
            for (const auto& code : input.trackingCodes("codes", 1, MAX_CODES))
            {
                codes.pushBack(code);
            }
        }
        input.validate();
        const auto loaded = m_useCases.process.execute(context.user(), id, codes);
        return platform::HttpResponse::created(
            Json {{"shipment", LogisticsJsonPresenter::shipmentSummary(*loaded[0])},
                  {"shipments", shipmentList(loaded)},
                  {"batch", SortingJsonPresenter::sortingBatch(m_useCases.batch.execute(context.user(), id))}},
            locationOf("/sorting-batches/" + std::to_string(id) + "/processed-items/" + loaded[0]->code()));
    }

    platform::HttpResponse SortingBatchController::giveBack(const RequestContext& context) const
    {
        auto input = InputValidator::parameters(context);
        const std::uint64_t id {input.requiredIdentifier("id")};
        const std::string code {input.trackingCode("code")};
        input.validate();
        const auto returned = m_useCases.giveBack.execute(context.user(), id, code);
        return platform::HttpResponse::ok(
            Json {{"shipment", LogisticsJsonPresenter::shipmentSummary(*returned[0])},
                  {"shipments", shipmentList(returned)},
                  {"batch", SortingJsonPresenter::sortingBatch(m_useCases.batch.execute(context.user(), id))}});
    }

    platform::HttpResponse SortingBatchController::search(const RequestContext& context) const
    {
        auto input = InputValidator::parameters(context);
        const std::uint64_t id {input.requiredIdentifier("id")};
        const std::string code {input.trackingCode("code")};
        const SearchMode mode {
            input.optionalTerm<SearchMode>("algorithm", searchMode, ALGORITHMS).value_or(SearchMode::Both)};
        input.validate();
        const auto& batch = m_useCases.batch.execute(context.user(), id);
        Json body {{"code", code}, {"batchSortedBy", Vocabulary::name(batch.sortedBy())}};
        if (mode == SearchMode::Linear)
        {
            body["linear"] =
                SortingJsonPresenter::batchSearch(m_useCases.linearSearch.execute(context.user(), id, code), batch);
        }
        else if (mode == SearchMode::Binary)
        {
            body["binary"] =
                SortingJsonPresenter::batchSearch(m_useCases.binarySearch.execute(context.user(), id, code), batch);
        }
        else
        {
            const auto comparison = m_useCases.compare.execute(context.user(), id, code);
            body["linear"] = SortingJsonPresenter::batchSearch(comparison.linear, batch);
            body["binary"] = comparison.binary.has_value()
                                 ? SortingJsonPresenter::batchSearch(*comparison.binary, batch)
                                 : Json(nullptr);
            body["binaryUnavailableReason"] = comparison.binaryUnavailableReason;
        }
        return platform::HttpResponse::ok(body);
    }

} // namespace hubflow::sorting::adapters
