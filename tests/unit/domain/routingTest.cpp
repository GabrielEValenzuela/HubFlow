#include "support/fixtures.hpp"

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/routing/domain/services/pumAgent.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using namespace hubflow::shared::domain;
namespace ds = hubflow::ds;
using namespace hubflow::routing::domain;

namespace
{
class RoutingFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hubflow::test::buildReferenceCity(map);
    }

    SearchTrace search(const SearchAlgorithm algorithm, const std::string& from, const std::string& to) const
    {
        return SearchStrategy::of(algorithm).search(map, map.require(from), map.require(to));
    }

    std::vector<std::string> pathOf(const SearchTrace& trace) const
    {
        std::vector<std::string> ids;
        for (std::size_t i = 0; i < trace.path.size(); ++i)
        {
            ids.push_back(map.node(trace.path[i]).id);
        }
        return ids;
    }

    double pathCost(const SearchTrace& trace) const
    {
        double total {0.0};
        for (std::size_t i = 1; i < trace.path.size(); ++i)
        {
            map.graph().forEachNeighbor(trace.path[i - 1],
                                        [&](const std::size_t to, const double weight)
                                        {
                                            if (to == trace.path[i])
                                            {
                                                total += weight;
                                            }
                                        });
        }
        return total;
    }

    RoadMap map;
};
} // namespace

TEST_F(RoutingFixture, MapRejectsEdgesCheaperThanStraightLine)
{
    EXPECT_THROW(map.connect("HUB", "N5", 1.0), business_error);
    EXPECT_THROW(map.connect("HUB", "ZZZ", 50.0), business_error);
    EXPECT_EQ(map.node(map.hub()).id, "HUB");
}

TEST_F(RoutingFixture, BreadthFirstFindsFewestHops)
{
    const SearchTrace trace = search(SearchAlgorithm::BreadthFirst, "HUB", "N5");
    EXPECT_TRUE(trace.metrics.solutionFound);
    EXPECT_EQ(trace.metrics.maxDepth, 3U);
    EXPECT_EQ(pathOf(trace).front(), "HUB");
    EXPECT_EQ(pathOf(trace).back(), "N5");
}

TEST_F(RoutingFixture, AStarIsOptimalAndNeverWorseThanOthers)
{
    const SearchTrace aStar = search(SearchAlgorithm::AStar, "HUB", "N5");
    for (const auto algorithm : ALL_ALGORITHMS)
    {
        const SearchTrace other = search(algorithm, "HUB", "N5");
        EXPECT_TRUE(other.metrics.solutionFound);
        EXPECT_LE(aStar.metrics.costTotal, other.metrics.costTotal + 1e-9);
        EXPECT_NEAR(other.metrics.costTotal, pathCost(other), 1e-9);
    }
    EXPECT_NEAR(aStar.metrics.costTotal, 16.8, 1e-9);
}

TEST_F(RoutingFixture, DepthFirstAndGreedyReturnValidPaths)
{
    for (const auto algorithm : {SearchAlgorithm::DepthFirst, SearchAlgorithm::GreedyBestFirst})
    {
        const SearchTrace trace = search(algorithm, "S3", "N4");
        const auto        path  = pathOf(trace);
        EXPECT_EQ(path.front(), "S3");
        EXPECT_EQ(path.back(), "N4");
        EXPECT_EQ(trace.steps.size(), trace.metrics.nodesExplored);
    }
}

TEST_F(RoutingFixture, TraceRecordsFrontierAndExploredPerStep)
{
    const SearchTrace trace = search(SearchAlgorithm::BreadthFirst, "HUB", "C1");
    ASSERT_EQ(trace.steps.size(), 2U);
    const TraceStep& first = trace.steps.head()->value;
    EXPECT_EQ(map.node(first.expanded).id, "HUB");
    EXPECT_EQ(first.frontier.size(), 4U);
    EXPECT_EQ(first.explored.size(), 1U);
    EXPECT_EQ(trace.metrics.statesVisited, 2U);
}

TEST_F(RoutingFixture, StartEqualsGoalAndUnreachableGoal)
{
    const SearchTrace same = search(SearchAlgorithm::AStar, "C2", "C2");
    EXPECT_TRUE(same.metrics.solutionFound);
    EXPECT_EQ(same.path.size(), 1U);
    EXPECT_DOUBLE_EQ(same.metrics.costTotal, 0.0);

    map.addNode(RoadNode {"X1", "Isla", false, logistics::domain::Zone::Sur, 30.0, 30.0});
    const SearchTrace unreachable = search(SearchAlgorithm::BreadthFirst, "HUB", "X1");
    EXPECT_FALSE(unreachable.metrics.solutionFound);
    EXPECT_EQ(unreachable.path.size(), 0U);
    EXPECT_EQ(unreachable.metrics.statesVisited, 15U);
}

TEST_F(RoutingFixture, DeliveryRoutePlansRoundTripAndResolvesStops)
{
    const PumAgent              agent {map, SearchStrategy::of(SearchAlgorithm::AStar)};
    ds::DynamicArray<RouteStop> stops;
    stops.pushBack(RouteStop {1, "PKG-1008", "N5", StopOutcome::Pending});
    stops.pushBack(RouteStop {2, "PKG-1002", "N2", StopOutcome::Pending});

    DeliveryRoute* pRoute = DeliveryRoute::assign(7, Courier {3, "Pum Uno"}, stops, agent, hubflow::test::actor());
    std::size_t    segments {0};
    pRoute->forEachSegment(
        [&segments](const SearchTrace&)
        {
            ++segments;
        });

    EXPECT_EQ(segments, 3U);
    EXPECT_EQ(pRoute->pendingEvents(), 1U);
    EXPECT_EQ(pRoute->status(), RouteStatus::InProgress);
    pRoute->resolveStop(1, StopOutcome::Delivered, hubflow::test::actor());
    EXPECT_THROW(pRoute->resolveStop(1, StopOutcome::Rescheduled, hubflow::test::actor()), business_error);
    EXPECT_THROW(pRoute->resolveStop(2, StopOutcome::Pending, hubflow::test::actor()), business_error);
    pRoute->resolveStop(2, StopOutcome::Rescheduled, hubflow::test::actor());
    EXPECT_EQ(pRoute->status(), RouteStatus::Completed);
    EXPECT_GT(pRoute->totalCostKm(), 0.0);
    delete pRoute;
}

TEST_F(RoutingFixture, DeliveryRouteRequiresStops)
{
    const PumAgent agent {map, SearchStrategy::of(SearchAlgorithm::BreadthFirst)};
    EXPECT_THROW(DeliveryRoute::assign(1, Courier {}, ds::DynamicArray<RouteStop> {}, agent, hubflow::test::actor()),
                 business_error);
}

TEST_F(RoutingFixture, ResolvingAStopWithTheSameOutcomeIsANoOp)
{
    const PumAgent              agent {map, SearchStrategy::of(SearchAlgorithm::AStar)};
    ds::DynamicArray<RouteStop> stops;
    stops.pushBack(RouteStop {1, "PKG-1008", "N5", StopOutcome::Pending});
    DeliveryRoute* pRoute = DeliveryRoute::assign(7, Courier {3, "Pum Uno"}, stops, agent, hubflow::test::actor());

    pRoute->resolveStop(1, StopOutcome::Delivered, hubflow::test::actor());
    const std::size_t eventsAfterFirst {pRoute->pendingEvents()};
    const RouteStop&  repeated = pRoute->resolveStop(1, StopOutcome::Delivered, hubflow::test::actor());

    EXPECT_EQ(repeated.outcome, StopOutcome::Delivered);
    EXPECT_EQ(pRoute->pendingEvents(), eventsAfterFirst);
    EXPECT_THROW(pRoute->resolveStop(1, StopOutcome::Rescheduled, hubflow::test::actor()), business_error);
    delete pRoute;
}
