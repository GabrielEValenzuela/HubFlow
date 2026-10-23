#include "support/applicationHarness.hpp"
#include "support/fixtures.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using shared::domain::business_error;
using shared::domain::ErrorKind;

namespace
{
std::vector<std::string> pendingCodes(const test::ApplicationHarness& app)
{
    std::vector<std::string> codes;
    app.listPendingShipments.execute(app.operador())
        .forEach(
            [&codes](const auto& shipment)
            {
                codes.push_back(shipment.code());
            });
    return codes;
}

ErrorKind kindOf(const std::function<void()>& action)
{
    ErrorKind kind {ErrorKind::Validation};
    try
    {
        action();
        ADD_FAILURE() << "se esperaba business_error";
    }
    catch (const business_error& error)
    {
        kind = error.kind();
    }
    return kind;
}
} // namespace

TEST(LogisticsFlow, RegisteringProjectsPersistsAndAudits)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    EXPECT_EQ(pendingCodes(app).front(), "PKG-1002");
    EXPECT_EQ(storage.shipments.states.size(), 8U);
    EXPECT_EQ(storage.shipments.movements.size(), 8U);
    EXPECT_EQ(storage.audit.entries.size(), 8U);
    EXPECT_EQ(storage.audit.entries.front().type, "shipment.registered");
}

TEST(LogisticsFlow, RejectsDuplicatesForeignDeliveryPointsAndForbiddenRoles)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    auto duplicate = test::initialDataset().front();
    auto wrongZone = test::draft("PKG-2000", "EXPRESS", "SUR");

    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.registerShipment.execute(app.operador(), duplicate);
                  }),
              ErrorKind::Conflict);
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.registerShipment.execute(app.operador(), wrongZone);
                  }),
              ErrorKind::Validation);
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.registerShipment.execute(app.pum(9), duplicate);
                  }),
              ErrorKind::Forbidden);
}

TEST(LogisticsFlow, DispatchRescheduleAndDeliverKeepPendingConsistent)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();

    const auto& dispatched = app.dispatchNextShipment.execute(app.operador(), "");
    EXPECT_EQ(dispatched.code(), "PKG-1002");
    EXPECT_EQ(pendingCodes(app).front(), "PKG-1006");

    app.rescheduleShipment.execute(app.operador(), "PKG-1002", "Destinatario ausente");
    EXPECT_EQ(pendingCodes(app)[1], "PKG-1002");
    EXPECT_EQ(storage.shipments.states.at("PKG-1002").attempts, 1U);

    app.dispatchNextShipment.execute(app.operador(), "");
    app.dispatchNextShipment.execute(app.operador(), "");
    app.deliverShipment.execute(app.operador(), "PKG-1002", "Entrega confirmada", true);
    EXPECT_EQ(app.getShipment.execute(app.operador(), "PKG-1002").history().size(), 5U);
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.rescheduleShipment.execute(app.operador(), "PKG-1002", "x");
                  }),
              ErrorKind::InvalidState);
    EXPECT_EQ(pendingCodes(app).size(), 6U);
}

TEST(LogisticsFlow, DispatchOnEmptyPendingIsConflict)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    EXPECT_EQ(kindOf(
                  [&]
                  {
                      app.dispatchNextShipment.execute(app.operador(), "");
                  }),
              ErrorKind::Conflict);
}

TEST(LogisticsFlow, LookupsOverAllStructuresAgree)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    for (const auto index : {logistics::application::LookupIndex::LinkedList,
                             logistics::application::LookupIndex::Bst,
                             logistics::application::LookupIndex::BPlusTree})
    {
        const auto result = app.findShipment.execute(app.operador(), "PKG-1005", index);
        ASSERT_NE(result.pShipment, nullptr);
        EXPECT_EQ(result.pShipment->recipient(), "Elena Castro");
    }
    const auto metrics = app.getOperationsMetrics.execute(app.operador());
    EXPECT_EQ(metrics.totalShipments, 8U);
    EXPECT_EQ(metrics.pendingByZone[1].summary.count, 3U);
}
