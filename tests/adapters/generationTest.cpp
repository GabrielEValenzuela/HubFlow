#include "support/fixtures.hpp"

#include "hubflow/logistics/adapters/out/generation/publicTrackingCodeGenerator.hpp"
#include "hubflow/logistics/adapters/out/generation/sequentialPkgCodeGenerator.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentId.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/logistics/domain/valueobjects/uuidText.hpp"
#include "hubflow/shared/adapters/out/generation/uuidV7IdentityGenerator.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <set>
#include <string>
#include <vector>

using namespace hubflow;
using logistics::adapters::PublicTrackingCodeGenerator;
using logistics::adapters::SequentialPkgCodeGenerator;
using logistics::domain::ShipmentCatalog;
using logistics::domain::TrackingCode;
using logistics::domain::UuidText;
using shared::adapters::UuidV7IdentityGenerator;

namespace
{
std::uint64_t millisecondsOf(const std::string& uuid)
{
    return std::stoull(uuid.substr(0, 8) + uuid.substr(9, 4), nullptr, 16);
}
} // namespace

TEST(UuidV7IdentityGenerator, ProducesRfc9562VersionSevenText)
{
    UuidV7IdentityGenerator generator;

    shared::application::IdentityGenerator& port = generator;
    const std::string                       id {port.next()};

    EXPECT_EQ(id.size(), 36U);
    EXPECT_TRUE(UuidText::isValid(id));
    EXPECT_EQ(id[14], '7');
    EXPECT_NE(std::string {"89ab"}.find(id[19]), std::string::npos);
}

TEST(UuidV7IdentityGenerator, EmbedsTheMillisecondTimestampInTheFirstFortyEightBits)
{
    UuidV7IdentityGenerator generator {[]
                                       {
                                           return std::uint64_t {1767225600123};
                                       }};

    shared::application::IdentityGenerator& port = generator;

    EXPECT_EQ(millisecondsOf(port.next()), 1767225600123U);
}

TEST(UuidV7IdentityGenerator, IdentitiesInTheSameMillisecondKeepIncreasingInTextOrder)
{
    UuidV7IdentityGenerator                 generator {[]
                                       {
                                           return std::uint64_t {1767225600123};
                                       }};
    shared::application::IdentityGenerator& port = generator;
    std::vector<std::string>                ids;
    for (int i = 0; i < 5000; ++i)
    {
        ids.push_back(port.next());
    }

    EXPECT_TRUE(std::is_sorted(ids.begin(), ids.end()));
    EXPECT_EQ(std::set<std::string>(ids.begin(), ids.end()).size(), ids.size());
    EXPECT_TRUE(std::all_of(ids.begin(), ids.end(), UuidText::isValid));
    EXPECT_GT(millisecondsOf(ids.back()), millisecondsOf(ids.front()));
}

TEST(UuidV7IdentityGenerator, LaterMillisecondsSortAfterEarlierOnes)
{
    std::uint64_t                           now {1767225600000};
    UuidV7IdentityGenerator                 generator {[&now]
                                       {
                                           return now;
                                       }};
    shared::application::IdentityGenerator& port = generator;
    const std::string                       first {port.next()};
    now += 5;
    const std::string second {port.next()};

    EXPECT_LT(first, second);
}

TEST(UuidV7IdentityGenerator, ABackwardsClockNeverReversesTheOrder)
{
    std::uint64_t                           now {1767225600500};
    UuidV7IdentityGenerator                 generator {[&now]
                                       {
                                           return now;
                                       }};
    shared::application::IdentityGenerator& port = generator;
    const std::string                       first {port.next()};
    now -= 400;
    const std::string second {port.next()};

    EXPECT_LT(first, second);
}

TEST(PublicTrackingCodeGenerator, ProducesCheckedCodesThatAreNotSequential)
{
    PublicTrackingCodeGenerator                    generator {42};
    ShipmentCatalog                                catalog;
    logistics::application::TrackingCodeGenerator& port = generator;
    std::set<std::string>                          codes;
    for (int i = 0; i < 500; ++i)
    {
        codes.insert(port.next(catalog));
    }

    EXPECT_EQ(codes.size(), 500U);
    for (const auto& code : codes)
    {
        EXPECT_EQ(code.size(), TrackingCode::PUBLIC_LENGTH);
        EXPECT_EQ(code.compare(0, 3, "HF-"), 0);
        EXPECT_TRUE(TrackingCode::isPublicFormat(code));
    }
}

TEST(PublicTrackingCodeGenerator, SkipsCodesAlreadyInUse)
{
    ShipmentCatalog                                catalog;
    hubflow::logistics::domain::TicketDispenser    tickets;
    PublicTrackingCodeGenerator                    first {7};
    logistics::application::TrackingCodeGenerator& port = first;
    const std::string                              taken {port.next(catalog)};
    auto                                           shipmentDraft = test::draft(taken, "EXPRESS");
    catalog.adopt(logistics::domain::Shipment::receive(shipmentDraft, tickets.next(), test::actor()));

    PublicTrackingCodeGenerator                    sameSeed {7};
    logistics::application::TrackingCodeGenerator& retrying = sameSeed;

    EXPECT_NE(retrying.next(catalog), taken);
}

TEST(SequentialPkgCodeGenerator, CountsUpAndSkipsTheNumbersInUse)
{
    ShipmentCatalog                             catalog;
    hubflow::logistics::domain::TicketDispenser tickets;
    catalog.adopt(
        logistics::domain::Shipment::receive(test::draft("PKG-1002", "EXPRESS"), tickets.next(), test::actor()));
    SequentialPkgCodeGenerator                     generator;
    logistics::application::TrackingCodeGenerator& port = generator;

    EXPECT_EQ(port.next(catalog), "PKG-1001");
    EXPECT_EQ(port.next(catalog), "PKG-1003");
}

TEST(SequentialPkgCodeGenerator, FailsWhenTheNumbersRunOut)
{
    ShipmentCatalog                                catalog;
    SequentialPkgCodeGenerator                     generator {SequentialPkgCodeGenerator::LAST_NUMBER};
    logistics::application::TrackingCodeGenerator& port = generator;

    EXPECT_EQ(port.next(catalog), "PKG-9999");
    EXPECT_THROW(port.next(catalog), shared::domain::business_error);
}
