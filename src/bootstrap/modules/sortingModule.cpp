#include "bootstrap/modules/sortingModule.hpp"

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/platform/http/apiRouter.hpp"
#include "hubflow/sorting/adapters/in/http/sortingBatchController.hpp"
#include "hubflow/sorting/application/usecases/commands/openSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/commands/prepareSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/commands/processSortingBatchItem.hpp"
#include "hubflow/sorting/application/usecases/commands/returnProcessedItem.hpp"
#include "hubflow/sorting/application/usecases/commands/sortSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/binarySearchInSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/compareSortingBatchSearches.hpp"
#include "hubflow/sorting/application/usecases/queries/getActiveSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/getSortingBatch.hpp"
#include "hubflow/sorting/application/usecases/queries/linearSearchInSortingBatch.hpp"

namespace hubflow::bootstrap
{

    struct SortingModule::Parts
    {
        Parts(sorting::domain::SortingDesk& desk, const logistics::application::LogisticsModel& logistics)
            : openSortingBatch {logistics, desk}
            , sortSortingBatch {desk}
            , prepareSortingBatch {desk}
            , processSortingBatchItem {desk, logistics.groups}
            , returnProcessedItem {desk, logistics.groups}
            , getActiveSortingBatch {desk}
            , getSortingBatch {desk}
            , linearSearch {desk}
            , binarySearch {desk}
            , compareSearches {desk}
        {
        }

        sorting::application::OpenSortingBatch openSortingBatch;
        sorting::application::SortSortingBatch sortSortingBatch;
        sorting::application::PrepareSortingBatch prepareSortingBatch;
        sorting::application::ProcessSortingBatchItem processSortingBatchItem;
        sorting::application::ReturnProcessedItem returnProcessedItem;
        sorting::application::GetActiveSortingBatch getActiveSortingBatch;
        sorting::application::GetSortingBatch getSortingBatch;
        sorting::application::LinearSearchInSortingBatch linearSearch;
        sorting::application::BinarySearchInSortingBatch binarySearch;
        sorting::application::CompareSortingBatchSearches compareSearches;
        std::unique_ptr<platform::Controller> spController;
    };

    SortingModule::SortingModule(sorting::domain::SortingDesk& desk,
                                 const logistics::application::LogisticsModel& logistics)
        : m_spParts {std::make_unique<Parts>(desk, logistics)}
    {
    }

    SortingModule::~SortingModule() = default;

    void SortingModule::registerRoutes(platform::ApiRouter& router)
    {
        Parts& parts = *m_spParts;
        parts.spController = std::make_unique<sorting::adapters::SortingBatchController>(
            sorting::adapters::SortingUseCases {parts.openSortingBatch,
                                                parts.sortSortingBatch,
                                                parts.prepareSortingBatch,
                                                parts.processSortingBatchItem,
                                                parts.returnProcessedItem,
                                                parts.getActiveSortingBatch,
                                                parts.getSortingBatch,
                                                parts.linearSearch,
                                                parts.binarySearch,
                                                parts.compareSearches});
        parts.spController->registerRoutes(router);
    }

} // namespace hubflow::bootstrap
