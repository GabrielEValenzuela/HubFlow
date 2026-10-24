#include "support/applicationHarness.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <gtest/gtest.h>

using namespace hubflow;
using shared::domain::BusinessError;
using shared::domain::ErrorKind;

namespace
{
    std::uint64_t openPreparedNorthBatch(test::ApplicationHarness& app)
    {
        const auto& batch = app.openSortingBatch.execute(app.operador(), "NORTE");
        app.sortSortingBatch.execute(app.operador(), batch.id(), sorting::domain::SortCriteria::CodeAscending);
        app.prepareSortingBatch.execute(app.operador(), batch.id());
        return batch.id();
    }
} // namespace

TEST(SortingFlow, ReturnProcessedItemGivesBackOnlyTheLastProcessedShipment)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto batchId = openPreparedNorthBatch(app);
    app.processSortingBatchItem.execute(app.operador(), batchId, {});
    const auto second = app.processSortingBatchItem.execute(app.operador(), batchId, {});
    const std::string lastCode {second[0]->code()};

    const auto returned = app.returnProcessedItem.execute(app.operador(), batchId, lastCode);

    EXPECT_EQ(returned[0]->code(), lastCode);
    EXPECT_EQ(app.getSortingBatch.execute(app.operador(), batchId).processed().size(), 1U);
}

TEST(SortingFlow, RepeatingReturnProcessedItemIsNotFoundAndChangesNothing)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto batchId = openPreparedNorthBatch(app);
    const std::string code {app.processSortingBatchItem.execute(app.operador(), batchId, {})[0]->code()};
    app.returnProcessedItem.execute(app.operador(), batchId, code);

    ErrorKind kind {ErrorKind::Validation};
    try
    {
        app.returnProcessedItem.execute(app.operador(), batchId, code);
    }
    catch (const BusinessError& error)
    {
        kind = error.errorKind();
    }

    EXPECT_EQ(kind, ErrorKind::NotFound);
    EXPECT_TRUE(app.getSortingBatch.execute(app.operador(), batchId).processed().isEmpty());
}

TEST(SortingFlow, SortingTwiceWithTheSameCriteriaKeepsTheSameOrder)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto& batch = app.openSortingBatch.execute(app.operador(), "NORTE");

    app.sortSortingBatch.execute(app.operador(), batch.id(), sorting::domain::SortCriteria::WeightDescending);
    const std::string firstOrder {batch.shipmentAt(0).code() + batch.shipmentAt(1).code() + batch.shipmentAt(2).code()};
    app.sortSortingBatch.execute(app.operador(), batch.id(), sorting::domain::SortCriteria::WeightDescending);

    EXPECT_EQ(batch.shipmentAt(0).code() + batch.shipmentAt(1).code() + batch.shipmentAt(2).code(), firstOrder);
}
