#include "datastructures/bitSet.hpp"
#include "datastructures/graph.hpp"
#include "datastructures/minHeap.hpp"
#include "datastructures/set.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    std::vector<std::string> items(const hubflow::ds::Set<std::string>& set)
    {
        std::vector<std::string> values;
        set.forEach([&values](const std::string& value) { values.push_back(value); });
        return values;
    }

    hubflow::ds::Set<std::string> setOf(std::initializer_list<const char*> values)
    {
        hubflow::ds::Set<std::string> set;
        for (const char* pValue : values)
        {
            set.insert(pValue);
        }
        return set;
    }

    struct AscendingIntPriority
    {
        bool operator()(const int first, const int second) const
        {
            return first < second;
        }
    };
} // namespace

TEST(BitSet, SetUnsetIsSetCountAndAnySet)
{
    hubflow::ds::BitSet<5> bits;
    EXPECT_FALSE(bits.isAnySet());
    EXPECT_EQ(bits.activeCount(), 0U);
    bits.set(0);
    bits.set(1);
    bits.set(2);
    EXPECT_TRUE(bits.isSet(2));
    EXPECT_FALSE(bits.isSet(3));
    EXPECT_EQ(bits.activeCount(), 3U);
    EXPECT_EQ(bits.toWord(), 0b00111U);
    bits.unset(1);
    EXPECT_EQ(bits.toWord(), 0b00101U);
    EXPECT_THROW(bits.set(5), std::out_of_range);
    EXPECT_THROW(static_cast<void>(bits.isSet(7)), std::out_of_range);
}

TEST(BitSet, BitwiseOperators)
{
    const auto first = hubflow::ds::BitSet<5>::fromWord(0b01111);
    const auto second = hubflow::ds::BitSet<5>::fromWord(0b10111);
    EXPECT_EQ((first & second).toWord(), 0b00111U);
    EXPECT_EQ((first | second).toWord(), 0b11111U);
    EXPECT_EQ((first ^ second).toWord(), 0b11000U);
    EXPECT_EQ(hubflow::ds::BitSet<5>::fromWord(0xFF).toWord(), 0b11111U);
}

TEST(Set, KeepsUniqueSortedElements)
{
    hubflow::ds::Set<std::string> set;
    EXPECT_TRUE(set.insert("PKG-1005"));
    EXPECT_TRUE(set.insert("PKG-1002"));
    EXPECT_FALSE(set.insert("PKG-1005"));
    EXPECT_TRUE(set.contains("PKG-1002"));
    EXPECT_FALSE(set.contains("PKG-1003"));
    EXPECT_EQ(items(set), (std::vector<std::string> {"PKG-1002", "PKG-1005"}));
    EXPECT_FALSE(set.remove("PKG-9999"));
    EXPECT_TRUE(set.remove("PKG-1002"));
    EXPECT_EQ(set.size(), 1U);
}

TEST(Set, UnionIntersectionAndDifference)
{
    const auto north = setOf({"PKG-1002", "PKG-1005", "PKG-1008"});
    const auto express = setOf({"PKG-1002", "PKG-1006"});
    EXPECT_EQ(items(north.intersect(express)), (std::vector<std::string> {"PKG-1002"}));
    EXPECT_EQ(items(north.unite(express)), (std::vector<std::string> {"PKG-1002", "PKG-1005", "PKG-1006", "PKG-1008"}));
    EXPECT_EQ(items(north.difference(express)), (std::vector<std::string> {"PKG-1005", "PKG-1008"}));
}

TEST(Set, EdgeCasesWithEmptyAndDisjointSets)
{
    const hubflow::ds::Set<std::string> emptySet;
    const auto firstSet = setOf({"PKG-1001", "PKG-1003"});
    const auto secondSet = setOf({"PKG-1002"});
    EXPECT_EQ(items(firstSet.unite(emptySet)), items(firstSet));
    EXPECT_TRUE(firstSet.intersect(secondSet).isEmpty());
    EXPECT_TRUE(emptySet.difference(firstSet).isEmpty());
    hubflow::ds::Set<std::string> copy {firstSet};
    copy.insert("PKG-1009");
    EXPECT_EQ(firstSet.size(), 2U);
    EXPECT_EQ(copy.size(), 3U);
}

TEST(MinHeap, PopsInPriorityOrder)
{
    hubflow::ds::MinHeap<int, AscendingIntPriority> heap;
    for (int value : {5, 1, 4, 2, 3})
    {
        heap.push(value);
    }
    std::vector<int> popped;
    while (!heap.isEmpty())
    {
        popped.push_back(heap.pop());
    }
    EXPECT_EQ(popped, (std::vector<int> {1, 2, 3, 4, 5}));
    EXPECT_THROW(heap.pop(), std::underflow_error);
}

TEST(Graph, StoresUndirectedWeightedEdges)
{
    hubflow::ds::Graph<std::string> graph;
    const auto harbor = graph.addVertex("Harbor");
    const auto junction = graph.addVertex("Junction");
    const auto depot = graph.addVertex("Depot");
    graph.addEdge(harbor, junction, 1.5);
    graph.addEdge(junction, depot, 2.0);

    std::vector<std::size_t> neighbours;
    graph.forEachNeighbor(junction,
                          [&neighbours](std::size_t destination, double) { neighbours.push_back(destination); });
    EXPECT_EQ(neighbours, (std::vector<std::size_t> {harbor, depot}));
    EXPECT_EQ(graph.edgeCount(), 2U);
    EXPECT_EQ(graph.findVertex([](const std::string& name) { return name == "Depot"; }), depot);
    EXPECT_FALSE(graph.findVertex([](const std::string& name) { return name == "Z"; }).has_value());
    EXPECT_THROW(graph.addEdge(harbor, harbor, 1.0), std::invalid_argument);
}
