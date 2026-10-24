#include "hubflow/routing/domain/services/search/searchStrategy.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::domain
{
namespace
{
const BreadthFirstSearch    gs_breadthFirst;
const DepthFirstSearch      gs_depthFirst;
const GreedyBestFirstSearch gs_greedyBestFirst;
const AStarSearch           gs_aStar;

constexpr double UNKNOWN_COST {-1.0};

class FrontierOwner final
{
public:
    explicit FrontierOwner(Frontier* pFrontier)
        : m_pFrontier {pFrontier}
    {
    }

    ~FrontierOwner()
    {
        delete m_pFrontier;
    }

    FrontierOwner(const FrontierOwner&)            = delete;
    FrontierOwner& operator=(const FrontierOwner&) = delete;

    Frontier& frontier() const noexcept
    {
        return *m_pFrontier;
    }

private:
    Frontier* m_pFrontier;
};

ds::DynamicArray<std::size_t> pathTo(const SearchNode* pGoal)
{
    ds::DynamicArray<std::size_t> path {pGoal->depth + 1, 0};
    for (const SearchNode* pNode = pGoal; pNode != nullptr; pNode = pNode->pParent)
    {
        path[pNode->depth] = pNode->vertex;
    }
    return path;
}

std::size_t countTrue(const ds::DynamicArray<bool>& flags)
{
    std::size_t count {0};
    for (std::size_t flagIndex = 0; flagIndex < flags.size(); ++flagIndex)
    {
        count += flags[flagIndex] ? 1 : 0;
    }
    return count;
}
} // namespace

const SearchStrategy& SearchStrategy::of(const SearchAlgorithm algorithm)
{
    const SearchStrategy* pStrategy = &gs_aStar;
    if (algorithm == SearchAlgorithm::BreadthFirst)
    {
        pStrategy = &gs_breadthFirst;
    }
    else if (algorithm == SearchAlgorithm::DepthFirst)
    {
        pStrategy = &gs_depthFirst;
    }
    else if (algorithm == SearchAlgorithm::GreedyBestFirst)
    {
        pStrategy = &gs_greedyBestFirst;
    }
    return *pStrategy;
}

SearchTrace SearchStrategy::search(const RoadMap& map, const std::size_t start, const std::size_t goal) const
{
    const std::size_t vertexCount {map.nodeCount()};
    if (start >= vertexCount || goal >= vertexCount)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation, "route.invalid_endpoints", "Origen o destino inexistente"};
    }
    SearchTrace trace;
    trace.algorithm = algorithm();
    trace.start     = start;
    trace.goal      = goal;

    SearchBookkeeping             bookkeeping {ds::DynamicArray<bool> {vertexCount, false},
                                   ds::DynamicArray<bool> {vertexCount, false},
                                   ds::DynamicArray<double> {vertexCount, UNKNOWN_COST}};
    ds::DynamicArray<std::size_t> exploredOrder;
    SearchNodePool                pool;
    const FrontierOwner           owner {createFrontier()};
    Frontier&                     frontier = owner.frontier();

    frontier.push(pool.create(start, nullptr, 0.0, priority(0.0, map.straightLine(start, goal))));
    bookkeeping.inFrontier[start] = true;
    bookkeeping.bestCost[start]   = 0.0;

    const SearchNode* pGoalNode = nullptr;
    while (!frontier.isEmpty() && pGoalNode == nullptr)
    {
        SearchNode* pNode = frontier.pop();
        if (!bookkeeping.explored[pNode->vertex])
        {
            bookkeeping.explored[pNode->vertex]   = true;
            bookkeeping.inFrontier[pNode->vertex] = false;
            exploredOrder.pushBack(pNode->vertex);
            ++trace.metrics.nodesExplored;
            if (pNode->vertex == goal)
            {
                pGoalNode = pNode;
            }
            else
            {
                expand(map, pNode, goal, frontier, bookkeeping, pool);
            }
            trace.steps.pushBack(TraceStep {
                trace.metrics.nodesExplored, pNode->vertex, frontier.snapshot(bookkeeping.explored), exploredOrder});
        }
    }

    trace.metrics.statesVisited   = countTrue(bookkeeping.explored);
    trace.metrics.nodesInFrontier = frontier.snapshot(bookkeeping.explored).size();
    if (pGoalNode != nullptr)
    {
        trace.path                  = pathTo(pGoalNode);
        trace.metrics.solutionFound = true;
        trace.metrics.maxDepth      = pGoalNode->depth;
        trace.metrics.costTotal     = pGoalNode->cost;
    }
    return trace;
}

void SearchStrategy::expand(const RoadMap&     map,
                            SearchNode*        pNode,
                            const std::size_t  goal,
                            Frontier&          frontier,
                            SearchBookkeeping& bookkeeping,
                            SearchNodePool&    pool) const
{
    map.graph().forEachNeighbor(
        pNode->vertex,
        [&](const std::size_t next, const double weight)
        {
            const double cost {pNode->cost + weight};
            if (!bookkeeping.explored[next] && admits(next, cost, bookkeeping))
            {
                bookkeeping.inFrontier[next] = true;
                bookkeeping.bestCost[next]   = cost;
                frontier.push(pool.create(next, pNode, cost, priority(cost, map.straightLine(next, goal))));
            }
        });
}

bool SearchStrategy::admits(const std::size_t vertex, const double, const SearchBookkeeping& bookkeeping) const noexcept
{
    return !bookkeeping.inFrontier[vertex];
}

SearchAlgorithm BreadthFirstSearch::algorithm() const noexcept
{
    return SearchAlgorithm::BreadthFirst;
}

Frontier* BreadthFirstSearch::createFrontier() const
{
    return new FifoFrontier {};
}

double BreadthFirstSearch::priority(const double, const double) const noexcept
{
    return 0.0;
}

SearchAlgorithm DepthFirstSearch::algorithm() const noexcept
{
    return SearchAlgorithm::DepthFirst;
}

Frontier* DepthFirstSearch::createFrontier() const
{
    return new LifoFrontier {};
}

double DepthFirstSearch::priority(const double, const double) const noexcept
{
    return 0.0;
}

SearchAlgorithm GreedyBestFirstSearch::algorithm() const noexcept
{
    return SearchAlgorithm::GreedyBestFirst;
}

Frontier* GreedyBestFirstSearch::createFrontier() const
{
    return new PriorityFrontier {};
}

double GreedyBestFirstSearch::priority(const double, const double heuristic) const noexcept
{
    return heuristic;
}

SearchAlgorithm AStarSearch::algorithm() const noexcept
{
    return SearchAlgorithm::AStar;
}

Frontier* AStarSearch::createFrontier() const
{
    return new PriorityFrontier {};
}

double AStarSearch::priority(const double cost, const double heuristic) const noexcept
{
    return cost + heuristic;
}

bool AStarSearch::admits(const std::size_t        vertex,
                         const double             cost,
                         const SearchBookkeeping& bookkeeping) const noexcept
{
    return bookkeeping.bestCost[vertex] < 0.0 || cost < bookkeeping.bestCost[vertex];
}

} // namespace hubflow::routing::domain
