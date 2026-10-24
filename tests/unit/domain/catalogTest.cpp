#include "support/fixtures.hpp"

#include "hubflow/logistics/domain/entities/serviceLevel.hpp"
#include "hubflow/logistics/domain/entities/shipmentFeature.hpp"
#include "hubflow/logistics/domain/entities/zone.hpp"
#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using namespace hubflow::logistics::domain;
using hubflow::shared::domain::BusinessError;
using hubflow::test::actor;

namespace
{
    class FakeOccupancy final : public PlantOccupancy
    {
    public:
        bool shipments {false};
        bool withService {false};
        bool routes {false};
        bool mapNodes {false};

    private:
        bool hasShipmentsInZone(const std::string&) const override
        {
            return shipments;
        }

        bool hasShipmentsWithService(const std::string&) const override
        {
            return withService;
        }

        bool hasRoutesInProgressInZone(const std::string&) const override
        {
            return routes;
        }

        bool hasActiveMapNodesInZone(const std::string&) const override
        {
            return mapNodes;
        }
    };

    class LowestFirstPolicy final : public PriorityPolicy
    {
    private:
        unsigned priorityOf(const Shipment& shipment) const override
        {
            return shipment.service() == "ESTANDAR" ? 9 : 1;
        }

        unsigned highestPriority() const override
        {
            return 9;
        }
    };

    std::string codeOfFailure(const std::function<void()>& action)
    {
        std::string code;
        try
        {
            action();
        }
        catch (const BusinessError& error)
        {
            code = error.code();
        }
        return code;
    }
} // namespace

TEST(CatalogCodeTest, AcceptsUppercaseDigitsAndUnderscoreOnly)
{
    EXPECT_TRUE(CatalogCode::isValid("OESTE"));
    EXPECT_TRUE(CatalogCode::isValid("ZONA_2"));
    EXPECT_FALSE(CatalogCode::isValid("oeste"));
    EXPECT_FALSE(CatalogCode::isValid("O"));
    EXPECT_FALSE(CatalogCode::isValid("UNA_ZONA_MUY_LARGA"));
    EXPECT_FALSE(CatalogCode::isValid("CON ESPACIO"));
    EXPECT_THROW(CatalogCode {"mal"}, BusinessError);
}

TEST(ZoneTest, OpensRenamesAndRaisesOneEventPerRealChange)
{
    Zone* pZone = Zone::open("OESTE", "Oeste", actor());
    EXPECT_EQ(pZone->pendingEvents(), 1U);
    EXPECT_TRUE(pZone->isActive());

    const FakeOccupancy free;
    pZone->revise(std::string {"Oeste"}, std::nullopt, free, actor());
    EXPECT_EQ(pZone->pendingEvents(), 1U);
    pZone->revise(std::string {"Zona Oeste"}, std::nullopt, free, actor());
    EXPECT_EQ(pZone->pendingEvents(), 2U);
    EXPECT_EQ(pZone->name(), "Zona Oeste");
    EXPECT_THROW(pZone->revise(std::string {"x"}, std::nullopt, free, actor()), BusinessError);
    EXPECT_EQ(pZone->name(), "Zona Oeste");
    delete pZone;
}

TEST(ZoneTest, DeactivationIsRefusedForEachReasonAndAppliedOnlyWhenFree)
{
    Zone* pZone = Zone::restore({"OESTE", "Oeste", true});
    FakeOccupancy occupancy;

    occupancy.shipments = true;
    EXPECT_EQ(codeOfFailure([&] { pZone->revise(std::nullopt, false, occupancy, actor()); }), "zone.has_shipments");
    occupancy.shipments = false;
    occupancy.routes = true;
    EXPECT_EQ(codeOfFailure([&] { pZone->revise(std::nullopt, false, occupancy, actor()); }), "zone.has_routes");
    occupancy.routes = false;
    occupancy.mapNodes = true;
    EXPECT_EQ(codeOfFailure([&] { pZone->revise(std::nullopt, false, occupancy, actor()); }), "zone.has_map_nodes");
    EXPECT_TRUE(pZone->isActive());
    EXPECT_EQ(pZone->pendingEvents(), 0U);

    occupancy.mapNodes = false;
    pZone->revise(std::nullopt, false, occupancy, actor());
    EXPECT_FALSE(pZone->isActive());
    EXPECT_EQ(pZone->pendingEvents(), 1U);
    pZone->revise(std::nullopt, false, occupancy, actor());
    EXPECT_EQ(pZone->pendingEvents(), 1U);
    pZone->revise(std::nullopt, true, occupancy, actor());
    EXPECT_TRUE(pZone->isActive());
    delete pZone;
}

TEST(ZoneTest, RenameAndDeactivateTogetherAreAllOrNothing)
{
    Zone* pZone = Zone::restore({"OESTE", "Oeste", true});
    FakeOccupancy occupancy;
    occupancy.shipments = true;
    EXPECT_THROW(pZone->revise(std::string {"Otro nombre"}, false, occupancy, actor()), BusinessError);
    EXPECT_EQ(pZone->name(), "Oeste");
    EXPECT_EQ(pZone->pendingEvents(), 0U);
    delete pZone;
}

TEST(ServiceLevelTest, ValidatesPriorityAndRefusesDeactivationWithShipmentsInPlant)
{
    EXPECT_THROW(ServiceLevel::open("RAPIDO", "Rápido", 0, actor()), BusinessError);
    EXPECT_THROW(ServiceLevel::open("RAPIDO", "Rápido", 1001, actor()), BusinessError);

    ServiceLevel* pService = ServiceLevel::open("RAPIDO", "Rápido", 5, actor());
    EXPECT_EQ(pService->priority(), 5U);
    FakeOccupancy occupancy;
    occupancy.withService = true;
    EXPECT_EQ(codeOfFailure([&] { pService->revise(std::nullopt, false, occupancy, actor()); }),
              "service_level.has_shipments");
    occupancy.withService = false;
    pService->revise(std::nullopt, false, occupancy, actor());
    EXPECT_FALSE(pService->isActive());
    delete pService;
}

TEST(ShipmentFeatureTest, TogglesWithoutRestrictions)
{
    ShipmentFeature* pFeature = ShipmentFeature::open("FRAGIL", "Frágil", actor());
    pFeature->revise(std::nullopt, false, actor());
    EXPECT_FALSE(pFeature->isActive());
    pFeature->revise(std::nullopt, false, actor());
    EXPECT_EQ(pFeature->pendingEvents(), 2U);
    delete pFeature;
}

TEST(ReferenceCatalogTest, RejectsDuplicatesAndReportsMissing)
{
    ZoneCatalog zones {CatalogNouns {"zone", "la zona", "una zona"}};
    zones.adopt(Zone::open("OESTE", "Oeste", actor()));
    EXPECT_EQ(codeOfFailure([&] { zones.adopt(Zone::open("OESTE", "Otra", actor())); }), "zone.duplicate");
    EXPECT_EQ(codeOfFailure([&] { zones.require("ESTE"); }), "zone.not_found");
    EXPECT_EQ(zones.size(), 1U);
    EXPECT_EQ(zones.find("ESTE"), nullptr);
}

TEST(PriorityPolicyTest, ANewServiceWithHigherPriorityGoesFirstWithoutTouchingTheList)
{
    test::ReferencePriorities priorities;
    priorities.services.adopt(ServiceLevel::restore({"RELAMPAGO", "Relámpago", 10, true}));
    ShipmentCatalog catalog;
    PendingShipments pending {priorities.policy};
    TicketDispenser tickets;
    for (const auto& [code, service] : std::vector<std::pair<std::string, std::string>> {
             {"PKG-0001", "EXPRESS"}, {"PKG-0002", "ESTANDAR"}, {"PKG-0003", "RELAMPAGO"}, {"PKG-0004", "RELAMPAGO"}})
    {
        pending.insert(&catalog.adopt(Shipment::receive(test::draft(code, service), tickets.next(), actor())));
    }

    std::vector<std::string> order;
    pending.forEach([&order](const Shipment& shipment) { order.push_back(shipment.code()); });
    EXPECT_EQ(order, (std::vector<std::string> {"PKG-0003", "PKG-0004", "PKG-0001", "PKG-0002"}));
    const PriorityPolicy& policy = priorities.policy;
    EXPECT_EQ(policy.highestPriority(), 10U);
}

TEST(PriorityPolicyTest, ThePendingListObeysWhateverPolicyItIsGiven)
{
    const LowestFirstPolicy policy;
    ShipmentCatalog catalog;
    PendingShipments pending {policy};
    TicketDispenser tickets;
    for (const auto& [code, service] : std::vector<std::pair<std::string, std::string>> {
             {"PKG-0001", "EXPRESS"}, {"PKG-0002", "ESTANDAR"}, {"PKG-0003", "EXPRESS"}})
    {
        pending.insert(&catalog.adopt(Shipment::receive(test::draft(code, service), tickets.next(), actor())));
    }

    std::vector<std::string> order;
    pending.forEach([&order](const Shipment& shipment) { order.push_back(shipment.code()); });
    EXPECT_EQ(order, (std::vector<std::string> {"PKG-0002", "PKG-0001", "PKG-0003"}));
}

TEST(ShipmentFeaturesTest, AShipmentCarriesZeroOrManyDistinctFeatures)
{
    TicketDispenser tickets;
    auto withFeatures = test::draft("PKG-0001", "EXPRESS");
    withFeatures.features.pushBack("FRAGIL");
    withFeatures.features.pushBack("REFRIGERADO");
    withFeatures.features.pushBack("FRAGIL");
    Shipment* pShipment = Shipment::receive(withFeatures, tickets.next(), actor());
    EXPECT_EQ(pShipment->features().size(), 2U);
    EXPECT_TRUE(pShipment->hasFeature("FRAGIL"));
    EXPECT_FALSE(pShipment->hasFeature("VOLUMINOSO"));
    EXPECT_EQ(pShipment->state().draft.features.size(), 2U);
    delete pShipment;

    Shipment* pPlain = Shipment::receive(test::draft("PKG-0002", "EXPRESS"), tickets.next(), actor());
    EXPECT_EQ(pPlain->features().size(), 0U);
    delete pPlain;
}

TEST(RoadMapGrowthTest, ANewPointJoinsTheNetworkAtomically)
{
    routing::domain::RoadMap map;
    test::buildReferenceCity(map);
    ds::DynamicArray<routing::domain::RoadLink> links;
    links.pushBack({"HUB", 4.5});
    const routing::domain::RoadNode west {"O1", "Plaza Oeste", false, "OESTE", 6.0, 10.0, true};

    const auto index = map.declareNode(west, links, actor());

    EXPECT_EQ(map.node(index).id, "O1");
    EXPECT_EQ(map.node(index).zone, "OESTE");
    EXPECT_EQ(map.pendingEvents(), 1U);
    EXPECT_EQ(map.graph().degree(map.require("HUB")), 5U);

    const auto nodes = map.nodeCount();
    EXPECT_THROW(map.declareNode(west, links, actor()), BusinessError);
    ds::DynamicArray<routing::domain::RoadLink> shortcut;
    shortcut.pushBack({"HUB", 1.0});
    EXPECT_EQ(
        codeOfFailure([&] { map.declareNode({"O2", "Atajo", false, "OESTE", 0.0, 10.0, true}, shortcut, actor()); }),
        "road_edge.inadmissible");
    EXPECT_EQ(codeOfFailure([&] { map.declareNode({"O3", "Isla", false, "OESTE", 0.0, 0.0, true}, {}, actor()); }),
              "road_node.unconnected");
    ds::DynamicArray<routing::domain::RoadLink> toNowhere;
    toNowhere.pushBack({"ZZZ", 50.0});
    EXPECT_THROW(map.declareNode({"O4", "Perdido", false, "OESTE", 0.0, 0.0, true}, toNowhere, actor()), BusinessError);
    EXPECT_EQ(map.nodeCount(), nodes);
}

TEST(RoadMapGrowthTest, ZoneAndAvailabilityChangesAreIdempotentAndTheHubStays)
{
    routing::domain::RoadMap map;
    test::buildReferenceCity(map);
    map.reviseNode("N1", std::string {"CENTRO"}, false, actor());
    EXPECT_EQ(map.node(map.require("N1")).zone, "CENTRO");
    EXPECT_FALSE(map.node(map.require("N1")).active);
    EXPECT_EQ(map.pendingEvents(), 1U);
    map.reviseNode("N1", std::string {"CENTRO"}, false, actor());
    EXPECT_EQ(map.pendingEvents(), 1U);
    EXPECT_EQ(codeOfFailure([&] { map.reviseNode("HUB", std::nullopt, false, actor()); }), "road_node.hub_required");
    EXPECT_THROW(map.reviseNode("NOPE", std::nullopt, false, actor()), BusinessError);
}
