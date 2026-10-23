#include "hubflow/logistics/domain/valueobjects/phone.hpp"
#include "hubflow/logistics/domain/valueobjects/postalCode.hpp"
#include "hubflow/logistics/domain/valueobjects/recipient.hpp"
#include "hubflow/routing/domain/services/travelTimePolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"
#include "hubflow/tracking/domain/services/customerTimeline.hpp"
#include "hubflow/tracking/domain/services/customerWording.hpp"
#include "hubflow/tracking/domain/services/deliveryWindow.hpp"
#include "hubflow/tracking/domain/services/verificationProof.hpp"

#include <gtest/gtest.h>

using namespace hubflow;
using logistics::domain::Phone;
using logistics::domain::PostalCode;
using logistics::domain::Recipient;
using shared::domain::business_error;
using tracking::domain::CustomerTimeline;
using tracking::domain::CustomerWording;
using tracking::domain::DeliveryWindowPolicy;
using tracking::domain::Milestone;
using tracking::domain::MilestoneEvent;
using tracking::domain::ProofCheck;
using tracking::domain::VerificationProof;

TEST(PhoneValueObject, KeepsOnlyTheDigitsOfTheWayPeopleWriteThem)
{
    EXPECT_EQ(Phone {"+54 (011) 4521-7788"}.digits(),
              "540114521"
              "7788");
    EXPECT_EQ(Phone {"4521 7788"}.digits(), "45217788");
    EXPECT_EQ(Phone {"4521.7788"}.lastFour(), "7788");
}

TEST(PhoneValueObject, AcceptsBetweenEightAndFifteenDigits)
{
    EXPECT_TRUE(Phone::accepts("12345678"));
    EXPECT_TRUE(Phone::accepts("123456789012345"));
    EXPECT_FALSE(Phone::accepts("1234567"));
    EXPECT_FALSE(Phone::accepts("1234567890123456"));
}

TEST(PhoneValueObject, RejectsLettersAndMisplacedPlusSigns)
{
    EXPECT_FALSE(Phone::accepts("11 4521-77ab"));
    EXPECT_FALSE(Phone::accepts("11+45217788"));
    EXPECT_FALSE(Phone::accepts(""));
    EXPECT_THROW(Phone {"abc"}, business_error);
}

TEST(PhoneValueObject, MaskingRevealsOnlyTheLastFourDigits)
{
    EXPECT_EQ(Phone {"11 5532-4410"}.masked(), "****4410");
}

TEST(PhoneValueObject, ProofIsExactlyFourDigits)
{
    EXPECT_TRUE(Phone::isProof("0042"));
    EXPECT_FALSE(Phone::isProof("042"));
    EXPECT_FALSE(Phone::isProof("00a2"));
    EXPECT_FALSE(Phone::isProof("00421"));
}

TEST(PostalCodeValueObject, AcceptsTheNumericAndTheCpaFormats)
{
    EXPECT_TRUE(PostalCode::accepts("1425"));
    EXPECT_TRUE(PostalCode::accepts("C1425ABC"));
    EXPECT_TRUE(PostalCode::accepts("c1425 abc"));
    EXPECT_EQ(PostalCode {"b1642 ejc"}.value(), "B1642EJC");
}

TEST(PostalCodeValueObject, RejectsAnythingElse)
{
    EXPECT_FALSE(PostalCode::accepts("142"));
    EXPECT_FALSE(PostalCode::accepts("14250"));
    EXPECT_FALSE(PostalCode::accepts("1425ABC"));
    EXPECT_FALSE(PostalCode::accepts("C1425AB"));
    EXPECT_FALSE(PostalCode::accepts("CC425ABC"));
    EXPECT_FALSE(PostalCode::accepts(""));
    EXPECT_THROW(PostalCode {"nope"}, business_error);
}

TEST(RecipientValueObject, ValidatesNamePhoneAndPostalCodeTogether)
{
    const Recipient recipient {"  Bruno Díaz ", "11 5532-4410", "1640"};

    EXPECT_EQ(recipient.name(), "Bruno Díaz");
    EXPECT_EQ(recipient.firstName(), "Bruno");
    EXPECT_EQ(recipient.phone().lastFour(), "4410");
    EXPECT_EQ(recipient.postalCode().value(), "1640");
    EXPECT_THROW((Recipient {"A", "11 5532-4410", "1640"}), business_error);
    EXPECT_THROW((Recipient {"Bruno", "1", "1640"}), business_error);
    EXPECT_THROW((Recipient {"Bruno", "11 5532-4410", "x"}), business_error);
}

TEST(CustomerWordingPhrases, UseTheCustomerLanguageOfTheJourney)
{
    EXPECT_STREQ(CustomerWording::sentence(Milestone::Received), "Recibimos tu paquete en el centro de distribución");
    EXPECT_STREQ(CustomerWording::sentence(Milestone::Prepared), "Tu paquete fue preparado para el reparto");
    EXPECT_STREQ(CustomerWording::sentence(Milestone::OnTheWay), "Está en camino");
    EXPECT_STREQ(CustomerWording::sentence(Milestone::RetryScheduled),
                 "No pudimos entregarlo, vamos a intentar de nuevo");
    EXPECT_STREQ(CustomerWording::sentence(Milestone::Delivered), "Entregado");
    EXPECT_EQ(CustomerWording::notice(Milestone::OnTheWay, "HF-X"), "Paquete HF-X: Está en camino");
}

TEST(CustomerTimelineMapping, KeepsTheOrderAndUsesTheCustomerPhrases)
{
    ds::DynamicArray<MilestoneEvent> events;
    events.pushBack(MilestoneEvent {Milestone::Received, 100});
    events.pushBack(MilestoneEvent {Milestone::OnTheWay, 200});
    events.pushBack(MilestoneEvent {Milestone::RetryScheduled, 300});

    const auto timeline = CustomerTimeline::from(events);

    ASSERT_EQ(timeline.size(), 3U);
    EXPECT_EQ(timeline.entry(0).text, "Recibimos tu paquete en el centro de distribución");
    EXPECT_EQ(timeline.entry(1).occurredAt, 200);
    EXPECT_EQ(timeline.current().milestone, Milestone::RetryScheduled);
    EXPECT_FALSE(timeline.finished());
}

TEST(CustomerTimelineMapping, IsFinishedOnlyWhenTheLastStepIsTheDelivery)
{
    ds::DynamicArray<MilestoneEvent> events;
    events.pushBack(MilestoneEvent {Milestone::OnTheWay, 1});
    events.pushBack(MilestoneEvent {Milestone::Delivered, 2});

    EXPECT_TRUE(CustomerTimeline::from(events).finished());
    EXPECT_TRUE(CustomerTimeline::from(ds::DynamicArray<MilestoneEvent> {}).empty());
}

TEST(DeliveryWindowEstimate, StartsAtTheQuarterHourBeforeTheArrivalAndLastsAnHour)
{
    constexpr std::int64_t now {1700000000};
    const auto             window = DeliveryWindowPolicy::around(now, 50);
    const std::int64_t     arrival {now + 50 * 60};

    EXPECT_EQ(window.from % DeliveryWindowPolicy::ROUNDING_SECONDS, 0);
    EXPECT_LE(window.from, arrival);
    EXPECT_GT(window.from + DeliveryWindowPolicy::ROUNDING_SECONDS, arrival);
    EXPECT_EQ(window.to - window.from, 3600);
}

TEST(DeliveryWindowEstimate, NeverStartsInThePast)
{
    constexpr std::int64_t now {1700000123};

    EXPECT_EQ(DeliveryWindowPolicy::around(now, 0).from, now);
}

TEST(TravelTimeEstimate, UsesTheSameAssumptionsAsThePlant)
{
    EXPECT_EQ(routing::domain::TravelTimePolicy::estimateMinutes(25.0, 0), 60U);
    EXPECT_EQ(routing::domain::TravelTimePolicy::estimateMinutes(12.5, 3), 42U);
}

TEST(VerificationProofCheck, NeedsAtLeastOneDataAndAllOfThemMustMatch)
{
    EXPECT_TRUE(ProofCheck::matches(VerificationProof {"4410", ""}, "4410", "1640"));
    EXPECT_TRUE(ProofCheck::matches(VerificationProof {"", "c1425 abc"}, "4410", "C1425ABC"));
    EXPECT_TRUE(ProofCheck::matches(VerificationProof {"4410", "1640"}, "4410", "1640"));
    EXPECT_FALSE(ProofCheck::matches(VerificationProof {"4410", "9999"}, "4410", "1640"));
    EXPECT_FALSE(ProofCheck::matches(VerificationProof {"", ""}, "4410", "1640"));
    EXPECT_FALSE(ProofCheck::matches(VerificationProof {"441", ""}, "4410", "1640"));
    EXPECT_FALSE(ProofCheck::matches(VerificationProof {"4410", ""}, "", ""));
}
