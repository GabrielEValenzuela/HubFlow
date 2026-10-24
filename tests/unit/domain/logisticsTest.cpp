#include "support/fixtures.hpp"

#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/logistics/domain/services/catalogPriorityPolicy.hpp"
#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/shipmentLookup.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"
#include "hubflow/logistics/domain/services/zoneReport.hpp"
#include "hubflow/shared/domain/businessError.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow::shared::domain;
using namespace hubflow::logistics::domain;
using hubflow::test::actor;
using hubflow::test::draft;

namespace
{
class RecordingPublisher final : public DomainEventPublisher, private DomainEventVisitor
{
public:
    std::vector<std::string> events;

private:
    void publish(const DomainEvent& event) override
    {
        event.accept(*this);
    }

    void on(const ShipmentRegistered&) override
    {
        events.emplace_back("registered");
    }

    void on(const ShipmentDispatched&) override
    {
        events.emplace_back("dispatched");
    }

    void on(const ShipmentRescheduled&) override
    {
        events.emplace_back("rescheduled");
    }

    void on(const ShipmentDelivered&) override
    {
        events.emplace_back("delivered");
    }

    void on(const ParcelDeliveryConfirmed&) override
    {
        events.emplace_back("parcel");
    }
};

class LogisticsFixture : public ::testing::Test
{
protected:
    LogisticsFixture()
        : pending {priorities.policy}
    {
    }

    Shipment& receive(const ShipmentDraft& shipmentDraft)
    {
        Shipment& shipment = catalog.adopt(Shipment::receive(shipmentDraft, tickets.next(), actor()));
        pending.insert(&shipment);
        return shipment;
    }

    std::vector<std::string> pendingCodes() const
    {
        std::vector<std::string> codes;
        pending.forEach(
            [&codes](const Shipment& shipment)
            {
                codes.push_back(shipment.code());
            });
        return codes;
    }

    hubflow::test::ReferencePriorities priorities;
    ShipmentCatalog                    catalog;
    PendingShipments                   pending;
    TicketDispenser                    tickets;
};

class PendingShipmentsTest : public LogisticsFixture
{
};

class ShipmentHistoryTest : public LogisticsFixture
{
};

class ShipmentLookupTest : public LogisticsFixture
{
};
} // namespace

TEST_F(PendingShipmentsTest, OrdersByPriority)
{
    for (const auto& shipmentDraft : hubflow::test::initialDataset())
    {
        receive(shipmentDraft);
    }
    EXPECT_EQ(pendingCodes(),
              (std::vector<std::string> {
                  "PKG-1002", "PKG-1006", "PKG-1003", "PKG-1005", "PKG-1008", "PKG-1001", "PKG-1004", "PKG-1007"}));
}

TEST_F(PendingShipmentsTest, KeepsArrivalOrderWithinSamePriority)
{
    receive(draft("PKG-0001", "EXPRESS"));
    receive(draft("PKG-0002", "PRIORITARIO"));
    receive(draft("PKG-0003", "ESTANDAR"));
    receive(draft("PKG-0004", "EXPRESS"));
    EXPECT_EQ(pendingCodes(), (std::vector<std::string> {"PKG-0001", "PKG-0004", "PKG-0002", "PKG-0003"}));
}

TEST_F(PendingShipmentsTest, DispatchRemovesNodeButKeepsShipment)
{
    Shipment& first = receive(draft("PKG-0001", "EXPRESS"));
    receive(draft("PKG-0002", "ESTANDAR"));

    Shipment* pPopped = pending.popFront();
    pPopped->dispatch("Asignado al móvil 12", actor());

    EXPECT_EQ(pPopped, &first);
    EXPECT_EQ(first.status(), ShipmentStatus::EnReparto);
    EXPECT_EQ(first.history().size(), 2U);
    EXPECT_EQ(first.history().latest()->note, "Asignado al móvil 12");
    EXPECT_FALSE(pending.contains(&first));
    EXPECT_EQ(&catalog.require("PKG-0001"), &first);
}

TEST_F(PendingShipmentsTest, RescheduleIncrementsAttemptsAndReinsertsByPriority)
{
    Shipment& express = receive(draft("PKG-0001", "EXPRESS"));
    receive(draft("PKG-0002", "EXPRESS"));
    receive(draft("PKG-0003", "ESTANDAR"));
    pending.remove(&express);
    express.dispatch("Salida", actor());
    express.reschedule("Destinatario ausente", tickets.next(), actor());
    pending.insert(&express);

    EXPECT_EQ(express.attempts(), 1U);
    EXPECT_EQ(express.status(), ShipmentStatus::Reprogramado);
    EXPECT_EQ(express.history().latest()->status, ShipmentStatus::Reprogramado);
    EXPECT_EQ(pendingCodes(), (std::vector<std::string> {"PKG-0002", "PKG-0001", "PKG-0003"}));
}

TEST_F(ShipmentHistoryTest, ReadsForwardAndBackward)
{
    Shipment& shipment = receive(draft("PKG-0001", "EXPRESS"));
    shipment.classify("Clasificado para zona NORTE", actor());
    shipment.dispatch("Asignado al móvil 12", actor());
    shipment.reschedule("Destinatario ausente", tickets.next(), actor());
    shipment.dispatch("Segundo intento", actor());
    shipment.confirmAllParcels(actor());
    shipment.deliver("Entrega confirmada", actor());

    std::vector<ShipmentStatus> forward;
    std::vector<ShipmentStatus> backward;
    shipment.history().forEachChronological(
        [&forward](const Movement& movement)
        {
            forward.push_back(movement.status);
        });
    shipment.history().forEachReverse(
        [&backward](const Movement& movement)
        {
            backward.push_back(movement.status);
        });

    const std::vector<ShipmentStatus> expected {ShipmentStatus::Recibido,
                                                ShipmentStatus::Clasificado,
                                                ShipmentStatus::EnReparto,
                                                ShipmentStatus::Reprogramado,
                                                ShipmentStatus::EnReparto,
                                                ShipmentStatus::Entregado};
    EXPECT_EQ(forward, expected);
    EXPECT_EQ(backward, std::vector<ShipmentStatus>(expected.rbegin(), expected.rend()));
    EXPECT_EQ(shipment.history().latest()->sequence, 6U);
}

TEST_F(PendingShipmentsTest, RecursiveZoneSummaryCountsWeightAndExpress)
{
    for (const auto& shipmentDraft : hubflow::test::initialDataset())
    {
        receive(shipmentDraft);
    }
    const ZoneSummary north = summarizeZone(pending.head(), "NORTE", priorities.policy);
    EXPECT_EQ(north.count, 3U);
    EXPECT_DOUBLE_EQ(north.totalWeightKg, 6.05);
    EXPECT_EQ(north.urgentCount, 1U);
    EXPECT_EQ(heaviestInZone(pending.head(), "CENTRO")->code(), "PKG-1007");
    EXPECT_EQ(summarizeZone(nullptr, "SUR", priorities.policy).count, 0U);
}

TEST_F(PendingShipmentsTest, HandlesEmptyDuplicatedAndSingleElementCases)
{
    EXPECT_EQ(pending.popFront(), nullptr);
    EXPECT_THROW(catalog.require("PKG-9999"), business_error);

    Shipment& only = receive(draft("PKG-0001", "ESTANDAR"));
    EXPECT_EQ(only.history().size(), 1U);
    EXPECT_THROW(catalog.adopt(Shipment::receive(draft("PKG-0001", "EXPRESS"), tickets.next(), actor())),
                 business_error);
    EXPECT_EQ(pending.popFront(), &only);
    EXPECT_TRUE(pending.isEmpty());
    receive(draft("PKG-0002", "EXPRESS"));
    EXPECT_EQ(pendingCodes(), (std::vector<std::string> {"PKG-0002"}));
}

TEST(Shipment, EnforcesStateMachineAndTracksStateBits)
{
    TicketDispenser tickets;
    Shipment*       pShipment = Shipment::receive(draft("PKG-0001", "EXPRESS"), tickets.next(), actor());
    EXPECT_THROW(pShipment->deliver("x", actor()), business_error);
    pShipment->dispatch("salida", actor());
    pShipment->reschedule("ausente", tickets.next(), actor());
    EXPECT_TRUE(pShipment->stateHistory().isSet(bitOf(ShipmentStatus::Reprogramado)));
    EXPECT_FALSE(pShipment->stateHistory().isSet(bitOf(ShipmentStatus::Clasificado)));
    EXPECT_EQ(pShipment->stateHistory().activeCount(), 3U);
    pShipment->dispatch("segundo intento", actor());
    pShipment->confirmAllParcels(actor());
    pShipment->deliver("ok", actor());
    EXPECT_THROW(pShipment->reschedule("otra vez", tickets.next(), actor()), business_error);
    delete pShipment;
}

TEST(Shipment, RaisesOneEventPerBehaviour)
{
    TicketDispenser    tickets;
    RecordingPublisher publisher;
    Shipment*          pShipment = Shipment::receive(draft("PKG-0001", "EXPRESS"), tickets.next(), actor());
    pShipment->dispatch("salida", actor());
    pShipment->reschedule("ausente", tickets.next(), actor());
    pShipment->dispatch("salida", actor());
    pShipment->confirmAllParcels(actor());
    pShipment->deliver("ok", actor());
    EXPECT_EQ(pShipment->pendingEvents(), 6U);
    pShipment->publishEvents(publisher);
    EXPECT_EQ(
        publisher.events,
        (std::vector<std::string> {"registered", "dispatched", "rescheduled", "dispatched", "parcel", "delivered"}));
    EXPECT_EQ(pShipment->pendingEvents(), 0U);
    delete pShipment;
}

TEST(Shipment, ValidatesValueObjects)
{
    TicketDispenser tickets;
    EXPECT_THROW(Shipment::receive(draft("PKG-1", "EXPRESS"), tickets.next(), actor()), business_error);
    EXPECT_THROW(Shipment::receive(draft("PKG-0001", "EXPRESS", "NORTE", 0.0), tickets.next(), actor()),
                 business_error);
    EXPECT_THROW(Shipment::receive(draft("PKG-0001", "EXPRESS", "NORTE", 51.0), tickets.next(), actor()),
                 business_error);
}

TEST_F(ShipmentLookupTest, StrategiesCompareVisitedNodes)
{
    for (const auto& shipmentDraft : hubflow::test::initialDataset())
    {
        receive(shipmentDraft);
    }
    const LinearShipmentLookup linear;
    const BstShipmentLookup    bst;
    const BPlusShipmentLookup  bPlus;
    const ShipmentLookup&      linearLookup = linear;
    const ShipmentLookup&      bstLookup    = bst;
    const ShipmentLookup&      bPlusLookup  = bPlus;

    EXPECT_EQ(linearLookup.find(catalog, "PKG-1008").visited, 8U);
    EXPECT_EQ(bstLookup.find(catalog, "PKG-1008").visited, 8U);
    EXPECT_EQ(bPlusLookup.find(catalog, "PKG-1008").visited, catalog.bPlusTree().levels());
    EXPECT_EQ(bstLookup.find(catalog, "PKG-1008").pShipment, &catalog.require("PKG-1008"));
    EXPECT_EQ(catalog.bst().height(), 7);
    EXPECT_EQ(bstLookup.find(catalog, "PKG-0000").pShipment, nullptr);
}
