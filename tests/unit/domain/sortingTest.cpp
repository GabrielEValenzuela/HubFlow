#include "support/fixtures.hpp"

#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"
#include "hubflow/shared/domain/businessError.hpp"
#include "hubflow/sorting/domain/services/batchSearch.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow::shared::domain;
using namespace hubflow::logistics::domain;
using namespace hubflow::sorting::domain;

namespace
{
class SortingFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* codes[] {"PKG-1009", "PKG-1002", "PKG-1005", "PKG-1008", "PKG-1001", "PKG-1004", "PKG-1007"};
        double      weight {1.0};
        for (const char* pCode : codes)
        {
            Shipment& shipment = catalog.adopt(
                Shipment::receive(hubflow::test::draft(pCode, ServiceLevel::Estandar, Zone::Norte, weight),
                                  tickets.next(),
                                  hubflow::test::actor()));
            pending.insert(&shipment);
            weight += 0.5;
        }
    }

    static std::vector<std::string> codesOf(const SortingBatch& batch)
    {
        std::vector<std::string> codes;
        for (std::size_t i = 0; i < batch.size(); ++i)
        {
            codes.push_back(batch.at(i).code());
        }
        return codes;
    }

    ShipmentCatalog  catalog;
    PendingShipments pending;
    TicketDispenser  tickets;
};

const BatchSearch& linear()
{
    static const LinearBatchSearch s_linear;
    return s_linear;
}

const BatchSearch& binary()
{
    static const BinaryBatchSearch s_binary;
    return s_binary;
}

class SortingBatchTest : public SortingFixture
{
};

class BatchSearchTest : public SortingFixture
{
};
} // namespace

TEST_F(SortingBatchTest, SortsByCodeAndByWeightKeepingReferences)
{
    SortingBatch batch {1, Zone::Norte, pending};
    batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));
    EXPECT_EQ(codesOf(batch),
              (std::vector<std::string> {
                  "PKG-1001", "PKG-1002", "PKG-1004", "PKG-1005", "PKG-1007", "PKG-1008", "PKG-1009"}));
    EXPECT_EQ(&batch.at(0), &catalog.require("PKG-1001"));

    batch.sort(ShipmentOrdering::of(SortCriteria::WeightDescending));
    EXPECT_EQ(batch.at(0).code(), "PKG-1007");
    EXPECT_EQ(batch.sortedBy(), SortCriteria::WeightDescending);
    for (std::size_t i = 1; i < batch.size(); ++i)
    {
        EXPECT_GE(batch.at(i - 1).weightKg(), batch.at(i).weightKg());
    }
}

TEST_F(SortingBatchTest, InsertionSortBestCaseIsLinear)
{
    SortingBatch batch {1, Zone::Norte, pending};
    batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));
    const SortStats sorted = batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));
    EXPECT_EQ(sorted.comparisons, batch.size() - 1);
    EXPECT_EQ(sorted.shifts, 0U);
}

TEST_F(SortingBatchTest, EmptyZoneIsReported)
{
    EXPECT_THROW((SortingBatch {1, Zone::Sur, pending}), business_error);
}

TEST(SortingBatchEdgeCases, SingleElementBatchSortsTrivially)
{
    ShipmentCatalog  catalog;
    PendingShipments pending;
    TicketDispenser  tickets;
    Shipment&        only = catalog.adopt(Shipment::receive(
        hubflow::test::draft("PKG-0001", ServiceLevel::Express), tickets.next(), hubflow::test::actor()));
    pending.insert(&only);
    SortingBatch    batch {1, Zone::Norte, pending};
    const SortStats stats = batch.sort(ShipmentOrdering::of(SortCriteria::WeightDescending));
    EXPECT_EQ(stats.comparisons, 0U);
    EXPECT_EQ(&batch.at(0), &only);
}

TEST_F(SortingBatchTest, PreparingAgainSkipsPackagesAlreadyLoaded)
{
    SortingBatch batch {1, Zone::Norte, pending};
    batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));
    batch.prepare();
    const std::size_t total = batch.preparation().size();
    batch.processNext();
    batch.processNext();

    batch.prepare();
    batch.prepare();

    EXPECT_EQ(batch.preparation().size(), total - 2U);
    EXPECT_EQ(batch.processed().size(), 2U);
    EXPECT_EQ(batch.preparation().front()->code(), "PKG-1004");
}

TEST_F(SortingBatchTest, PreparationQueueIsFifoAndProcessedStackIsLifo)
{
    SortingBatch batch {1, Zone::Norte, pending};
    batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));
    batch.prepare();
    EXPECT_EQ(batch.preparation().front()->code(), "PKG-1001");
    EXPECT_EQ(batch.preparation().rear()->code(), "PKG-1009");

    EXPECT_EQ(batch.processNext().code(), "PKG-1001");
    EXPECT_EQ(batch.processNext().code(), "PKG-1002");
    EXPECT_EQ(batch.processNext().code(), "PKG-1004");
    EXPECT_EQ(batch.processed().top()->code(), "PKG-1004");
    EXPECT_EQ(batch.returnProcessed("PKG-1004").code(), "PKG-1004");
    EXPECT_EQ(batch.processed().top()->code(), "PKG-1002");

    while (!batch.preparation().isEmpty())
    {
        batch.processNext();
    }
    EXPECT_THROW(batch.processNext(), business_error);
    const auto unloaded = batch.unloadProcessed();
    EXPECT_EQ(unloaded[0]->code(), "PKG-1009");
    EXPECT_TRUE(batch.processed().isEmpty());
    EXPECT_THROW(batch.returnProcessed("PKG-1009"), business_error);
}

TEST_F(BatchSearchTest, LinearAndBinaryReportComparisons)
{
    SortingBatch batch {1, Zone::Norte, pending};
    EXPECT_THROW(binary().search(batch, "PKG-1005"), business_error);
    batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));

    EXPECT_EQ(linear().search(batch, "PKG-1001").comparisons, 1U);
    EXPECT_EQ(linear().search(batch, "PKG-1009").comparisons, 7U);
    EXPECT_EQ(binary().search(batch, "PKG-1005").comparisons, 1U);
    EXPECT_EQ(binary().search(batch, "PKG-1009").comparisons, 3U);
    EXPECT_EQ(*binary().search(batch, "PKG-1009").position, 6U);

    const auto missing = binary().search(batch, "PKG-1003");
    EXPECT_FALSE(missing.position.has_value());
    EXPECT_LE(missing.comparisons, 3U);
    EXPECT_FALSE(linear().search(batch, "PKG-1003").position.has_value());
    EXPECT_EQ(linear().search(batch, "PKG-1003").comparisons, 7U);

    batch.sort(ShipmentOrdering::of(SortCriteria::WeightDescending));
    EXPECT_TRUE(linear().search(batch, "PKG-1005").position.has_value());
    EXPECT_THROW(binary().search(batch, "PKG-1005"), business_error);
}

TEST_F(SortingBatchTest, DeskReplacesActiveBatch)
{
    SortingDesk desk;
    const auto& first = desk.open(Zone::Norte, pending);
    EXPECT_EQ(first.id(), 1U);
    const auto& second = desk.open(Zone::Norte, pending);
    EXPECT_EQ(second.id(), 2U);
    EXPECT_THROW(desk.require(1), business_error);
    desk.close();
    EXPECT_EQ(desk.active(), nullptr);
}

TEST_F(SortingBatchTest, ReturnProcessedItemOnlyAcceptsTheLastProcessedCode)
{
    SortingBatch batch {1, Zone::Norte, pending};
    batch.sort(ShipmentOrdering::of(SortCriteria::CodeAscending));
    batch.prepare();
    batch.processNext();
    batch.processNext();

    EXPECT_THROW(batch.returnProcessed("PKG-1001"), business_error);
    EXPECT_EQ(batch.processed().size(), 2U);
    EXPECT_EQ(batch.returnProcessed("PKG-1002").code(), "PKG-1002");
    EXPECT_EQ(batch.processed().top()->code(), "PKG-1001");
}

TEST_F(SortingBatchTest, ReturningTheSameItemTwiceFailsWithoutSideEffects)
{
    SortingBatch batch {1, Zone::Norte, pending};
    batch.prepare();
    batch.processNext();
    const std::string code {batch.processed().top()->code()};

    batch.returnProcessed(code);
    ErrorKind kind {ErrorKind::Validation};
    try
    {
        batch.returnProcessed(code);
    }
    catch (const business_error& error)
    {
        kind = error.kind();
    }

    EXPECT_EQ(kind, ErrorKind::NotFound);
    EXPECT_TRUE(batch.processed().isEmpty());
}
