#include "hubflow/sorting/adapters/in/http/sortingJsonPresenter.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::sorting::adapters
{
    namespace
    {
        using logistics::adapters::LogisticsJsonPresenter;
        using platform::Json;
        using shared::application::Vocabulary;
    } // namespace

    Json SortingJsonPresenter::sortingBatch(const domain::SortingBatch& batch)
    {
        Json items = Json::array();
        Json preparation = Json::array();
        Json processed = Json::array();
        for (std::size_t i = 0; i < batch.size(); ++i)
        {
            items.push_back(LogisticsJsonPresenter::shipmentSummary(batch.shipmentAt(i)));
        }
        batch.preparation().forEach([&preparation](const logistics::domain::Shipment* pShipment)
                                    { preparation.push_back(pShipment->code()); });
        batch.processed().forEach([&processed](const logistics::domain::Shipment* pShipment)
                                  { processed.push_back(pShipment->code()); });
        return Json {{"id", batch.id()},
                     {"zone", batch.zone()},
                     {"size", batch.size()},
                     {"sortedBy", Vocabulary::name(batch.sortedBy())},
                     {"items", items},
                     {"preparationQueue", preparation},
                     {"processedStack", processed}};
    }

    Json SortingJsonPresenter::sortStats(const domain::SortStats& stats)
    {
        return Json {{"algorithm", "INSERTION_SORT"}, {"comparisons", stats.comparisons}, {"shifts", stats.shifts}};
    }

    Json SortingJsonPresenter::batchSearch(const domain::BatchSearchOutcome& outcome, const domain::SortingBatch& batch)
    {
        Json steps = Json::array();
        outcome.steps.forEach(
            [&steps](const domain::BinarySearchStep& step)
            {
                steps.push_back(Json {{"left", step.left},
                                      {"right", step.right},
                                      {"middle", step.middle},
                                      {"codeAtMiddle", step.codeAtMiddle}});
            });
        return Json {{"found", outcome.position.has_value()},
                     {"position", outcome.position.has_value() ? Json(*outcome.position) : Json(nullptr)},
                     {"shipment",
                      outcome.position.has_value()
                          ? LogisticsJsonPresenter::shipmentSummary(batch.shipmentAt(*outcome.position))
                          : Json(nullptr)},
                     {"comparisons", outcome.comparisons},
                     {"steps", steps}};
    }

} // namespace hubflow::sorting::adapters
