#include "support/fixtures.hpp"

#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/logistics/domain/events/shipmentGroupEvents.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/shipmentGroupBook.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"
#include "hubflow/logistics/domain/valueobjects/dimensions.hpp"
#include "hubflow/logistics/domain/valueobjects/publicCodeCheck.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentId.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <gtest/gtest.h>
#include <string>

using namespace hubflow::logistics::domain;
using hubflow::shared::domain::business_error;
using hubflow::shared::domain::ErrorKind;
using hubflow::test::actor;
using hubflow::test::draft;

namespace
{
ShipmentDraft threeParcels(const std::string& code, const std::string& point = "N1")
{
    ShipmentDraft shipment = draft(code, "EXPRESS");
    shipment.deliveryPoint = point;
    shipment.parcels.clear();
    shipment.parcels.pushBack(ParcelData {2.0, 30.0, 20.0, 10.0});
    shipment.parcels.pushBack(ParcelData {1.0, 60.0, 50.0, 40.0});
    shipment.parcels.pushBack(ParcelData {0.5, 10.0, 10.0, 10.0});
    return shipment;
}

class GroupFixture : public ::testing::Test
{
protected:
    Shipment& receive(const ShipmentDraft& shipmentDraft)
    {
        return catalog.adopt(Shipment::receive(shipmentDraft, tickets.next(), actor()));
    }

    hubflow::ds::DynamicArray<std::string> codes(const std::string& first, const std::string& second)
    {
        hubflow::ds::DynamicArray<std::string> list;
        list.pushBack(first);
        list.pushBack(second);
        return list;
    }

    ShipmentCatalog   catalog;
    ShipmentGroupBook groups;
    TicketDispenser   tickets;
};
} // namespace

TEST(Dimensions, VolumetricWeightIsLengthTimesWidthTimesHeightOverFiveThousand)
{
    const Dimensions box {50.0, 40.0, 30.0};

    EXPECT_DOUBLE_EQ(box.volumetricWeightKg(), 12.0);
}

TEST(Dimensions, RejectsMeasuresOutOfRange)
{
    EXPECT_THROW((Dimensions {0.0, 10.0, 10.0}), business_error);
    EXPECT_THROW((Dimensions {10.0, -1.0, 10.0}), business_error);
    EXPECT_THROW((Dimensions {10.0, 10.0, 301.0}), business_error);
    EXPECT_NO_THROW((Dimensions {300.0, 0.1, 10.0}));
}

TEST(Parcel, LabelNamesTheShipmentTheOrdinalAndTheTotal)
{
    const Parcel parcel {"PKG-1001", 2, 3, ParcelData {1.0, 10.0, 10.0, 10.0}};

    EXPECT_EQ(parcel.label(), "PKG-1001-2/3");
}

TEST(Parcel, BillableWeightIsTheGreaterOfRealAndVolumetric)
{
    const Parcel light {"PKG-1001", 1, 2, ParcelData {1.0, 50.0, 40.0, 30.0}};
    const Parcel heavy {"PKG-1001", 2, 2, ParcelData {20.0, 10.0, 10.0, 10.0}};

    EXPECT_DOUBLE_EQ(light.billableWeightKg(), 12.0);
    EXPECT_DOUBLE_EQ(heavy.billableWeightKg(), 20.0);
}

TEST_F(GroupFixture, ShipmentWeightIsTheSumOfBillableParcelWeights)
{
    Shipment& shipment = receive(threeParcels("PKG-0001"));

    EXPECT_EQ(shipment.parcelCount(), 3U);
    EXPECT_DOUBLE_EQ(shipment.realWeightKg(), 3.5);
    EXPECT_DOUBLE_EQ(shipment.billableWeightKg(), 2.0 + 24.0 + 0.5);
    EXPECT_DOUBLE_EQ(shipment.weightKg(), shipment.billableWeightKg());
    EXPECT_EQ(shipment.parcel(1).label(), "PKG-0001-2/3");
}

TEST_F(GroupFixture, ShipmentNeedsBetweenOneAndTwentyParcels)
{
    ShipmentDraft none = threeParcels("PKG-0001");
    none.parcels.clear();
    ShipmentDraft many = threeParcels("PKG-0002");
    for (int i = 0; i < 18; ++i)
    {
        many.parcels.pushBack(ParcelData {1.0, 10.0, 10.0, 10.0});
    }
    ShipmentDraft twenty = threeParcels("PKG-0003");
    for (int i = 0; i < 17; ++i)
    {
        twenty.parcels.pushBack(ParcelData {1.0, 10.0, 10.0, 10.0});
    }

    EXPECT_THROW(Shipment::receive(none, tickets.next(), actor()), business_error);
    EXPECT_THROW(Shipment::receive(many, tickets.next(), actor()), business_error);
    EXPECT_EQ(receive(twenty).parcelCount(), 20U);
}

TEST_F(GroupFixture, ShipmentIdMustBeAUuidVersionSeven)
{
    ShipmentDraft bad = threeParcels("PKG-0001");
    bad.id            = "not-a-uuid";

    EXPECT_THROW(Shipment::receive(bad, tickets.next(), actor()), business_error);
    EXPECT_FALSE(ShipmentId::isValid("018f0000-0000-4000-8000-000000000001"));
    EXPECT_TRUE(ShipmentId::isValid("018f0000-0000-7000-8000-000000000001"));
}

TEST_F(GroupFixture, DeliveryRequiresEveryParcelConfirmed)
{
    Shipment& shipment = receive(threeParcels("PKG-0001"));
    shipment.dispatch("salida", actor());
    shipment.confirmParcel(1, actor());
    shipment.confirmParcel(2, actor());

    EXPECT_THROW(shipment.deliver("entregado", actor()), business_error);
    shipment.confirmParcel(3, actor());
    shipment.deliver("entregado", actor());

    EXPECT_EQ(shipment.status(), ShipmentStatus::Entregado);
}

TEST_F(GroupFixture, ConfirmingAParcelTwiceRaisesOneEventOnly)
{
    Shipment& shipment = receive(threeParcels("PKG-0001"));
    shipment.dispatch("salida", actor());
    const auto before = shipment.pendingEvents();

    shipment.confirmParcel(2, actor());
    shipment.confirmParcel(2, actor());

    EXPECT_EQ(shipment.pendingEvents(), before + 1);
    EXPECT_EQ(shipment.pendingParcelCount(), 2U);
}

TEST_F(GroupFixture, ParcelsCanOnlyBeConfirmedWhileOutForDelivery)
{
    Shipment& shipment = receive(threeParcels("PKG-0001"));

    EXPECT_THROW(shipment.confirmParcel(1, actor()), business_error);
    shipment.dispatch("salida", actor());
    EXPECT_THROW(shipment.confirmParcel(4, actor()), business_error);
    EXPECT_THROW(shipment.confirmParcel(0, actor()), business_error);
}

TEST_F(GroupFixture, PartialDeliveryKeepsDeliveredParcelsRecordedWhenRescheduled)
{
    Shipment& shipment = receive(threeParcels("PKG-0001"));
    shipment.dispatch("salida", actor());
    shipment.confirmParcel(1, actor());
    shipment.confirmParcel(3, actor());

    shipment.reschedule("faltó un bulto", tickets.next(), actor());
    const ShipmentState state     = shipment.state();
    Shipment*           pRestored = Shipment::restore(state);

    EXPECT_EQ(shipment.status(), ShipmentStatus::Reprogramado);
    EXPECT_TRUE(shipment.parcel(0).isDelivered());
    EXPECT_FALSE(shipment.parcel(1).isDelivered());
    EXPECT_TRUE(pRestored->parcel(2).isDelivered());
    EXPECT_EQ(pRestored->pendingParcelCount(), 1U);
    pRestored->dispatch("segundo intento", actor());
    EXPECT_THROW(pRestored->deliver("x", actor()), business_error);
    pRestored->confirmParcel(2, actor());
    pRestored->deliver("ok", actor());
    delete pRestored;
}

TEST(TrackingCode, AcceptsTheSequentialAndThePublicFormats)
{
    const std::string body {"7K3M9Q2X"};
    const std::string publicCode {std::string {"HF-"} + body + PublicCodeCheck::checkSymbolOf(body)};

    EXPECT_TRUE(TrackingCode::isValid("PKG-1001"));
    EXPECT_TRUE(TrackingCode::isValid(publicCode));
    EXPECT_TRUE(TrackingCode::isPublicFormat(publicCode));
    EXPECT_FALSE(TrackingCode::isValid("PKG-10x1"));
    EXPECT_FALSE(TrackingCode::isValid("HF-7K3M9Q2X"));
}

TEST(TrackingCode, CheckSymbolDetectsATypoAndASwap)
{
    const std::string body {"7K3M9Q2X"};
    const char        check {PublicCodeCheck::checkSymbolOf(body)};
    ASSERT_NE(check, '\0');
    const std::string good {std::string {"HF-"} + body + check};
    std::string       typo {good};
    typo[4] = typo[4] == 'A' ? 'B' : 'A';
    std::string swapped {good};
    std::swap(swapped[3], swapped[4]);

    EXPECT_TRUE(TrackingCode::isValid(good));
    EXPECT_FALSE(TrackingCode::isValid(typo));
    EXPECT_FALSE(TrackingCode::isValid(swapped));
    EXPECT_THROW(TrackingCode {typo}, business_error);
}

TEST(TrackingCode, OrdersLexicographicallyWithinAndAcrossFormats)
{
    EXPECT_TRUE(TrackingCode {"PKG-1001"} < TrackingCode {"PKG-1002"});
    const std::string  body {"00000000"};
    const TrackingCode first {std::string {"HF-"} + body + PublicCodeCheck::checkSymbolOf(body)};

    EXPECT_TRUE(first < TrackingCode {"PKG-1001"});
}

TEST_F(GroupFixture, GroupNeedsAtLeastTwoDistinctShipmentsAtTheSameDeliveryPoint)
{
    receive(threeParcels("PKG-0001", "N2"));
    receive(threeParcels("PKG-0002", "N2"));
    receive(threeParcels("PKG-0003", "N3"));

    EXPECT_THROW(groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0003"), catalog, actor()),
                 business_error);
    EXPECT_THROW(groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0001"), catalog, actor()),
                 business_error);
    EXPECT_THROW(groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0009"), catalog, actor()),
                 business_error);
    ShipmentGroup& group =
        groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0002"), catalog, actor());

    EXPECT_EQ(group.deliveryPoint(), "N2");
    EXPECT_EQ(groups.groupOf("PKG-0002"), &group);
    EXPECT_EQ(groups.groupOf("PKG-0003"), nullptr);
    EXPECT_EQ(group.pendingEvents(), 1U);
}

TEST_F(GroupFixture, AShipmentBelongsToOneGroupAtMost)
{
    receive(threeParcels("PKG-0001", "N2"));
    receive(threeParcels("PKG-0002", "N2"));
    receive(threeParcels("PKG-0003", "N2"));
    groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0002"), catalog, actor());

    EXPECT_THROW(groups.form("018f0000-0000-7000-8000-00000000000b", codes("PKG-0002", "PKG-0003"), catalog, actor()),
                 business_error);
}

TEST_F(GroupFixture, OnlyShipmentsAwaitingDispatchCanBeGrouped)
{
    receive(threeParcels("PKG-0001", "N2"));
    Shipment& outForDelivery = receive(threeParcels("PKG-0002", "N2"));
    outForDelivery.dispatch("salida", actor());

    EXPECT_THROW(groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0002"), catalog, actor()),
                 business_error);
}

TEST_F(GroupFixture, AGroupInDeliveryCannotBeDissolved)
{
    receive(threeParcels("PKG-0001", "N2"));
    Shipment&      second = receive(threeParcels("PKG-0002", "N2"));
    ShipmentGroup& group =
        groups.form("018f0000-0000-7000-8000-00000000000a", codes("PKG-0001", "PKG-0002"), catalog, actor());
    EXPECT_NO_THROW(groups.ensureDissolvable(group, catalog));

    second.dispatch("salida", actor());

    EXPECT_THROW(groups.ensureDissolvable(group, catalog), business_error);
    second.reschedule("ausente", tickets.next(), actor());
    groups.discard(group.id());
    EXPECT_EQ(groups.size(), 0U);
    EXPECT_EQ(groups.groupOf("PKG-0001"), nullptr);
}
