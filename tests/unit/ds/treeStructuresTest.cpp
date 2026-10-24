#include "datastructures/bPlusTree.hpp"
#include "datastructures/binarySearchTree.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace
{
    struct Payload
    {
        int value;
    };

    std::string code(const int number)
    {
        return "PKG-" + std::to_string(number);
    }
} // namespace

TEST(BinarySearchTree, InsertsRejectsDuplicatesAndTraversesInOrder)
{
    std::vector<Payload> payloads(8);
    hubflow::ds::BinarySearchTree<std::string, Payload> tree;
    const int order[] {1004, 1002, 1007, 1001, 1005, 1008, 1003, 1006};
    for (std::size_t i = 0; i < 8; ++i)
    {
        payloads[i].value = order[i];
        EXPECT_TRUE(tree.insert(code(order[i]), &payloads[i]));
    }
    EXPECT_FALSE(tree.insert(code(1005), &payloads[0]));

    std::vector<std::string> keys;
    tree.inOrder([&keys](const std::string& key, Payload*) { keys.push_back(key); });
    EXPECT_EQ(keys.front(), "PKG-1001");
    EXPECT_EQ(keys.back(), "PKG-1008");
    EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
    EXPECT_EQ(tree.size(), 8U);
    EXPECT_EQ(tree.height(), 3);
}

TEST(BinarySearchTree, FindReportsPathAndVisitedNodes)
{
    Payload a {1}, b {2}, c {3};
    hubflow::ds::BinarySearchTree<std::string, Payload> tree;
    tree.insert("PKG-1004", &a);
    tree.insert("PKG-1007", &b);
    tree.insert("PKG-1005", &c);

    std::vector<hubflow::ds::BstStep> steps;
    const auto found =
        tree.find("PKG-1005", [&steps](const std::string&, hubflow::ds::BstStep step) { steps.push_back(step); });
    EXPECT_EQ(found.pValue, &c);
    EXPECT_EQ(found.visited, 3U);
    EXPECT_EQ(steps,
              (std::vector<hubflow::ds::BstStep> {
                  hubflow::ds::BstStep::Right, hubflow::ds::BstStep::Left, hubflow::ds::BstStep::Found}));

    const auto missing = tree.find("PKG-9999");
    EXPECT_EQ(missing.pValue, nullptr);
    EXPECT_EQ(missing.visited, 2U);
}

TEST(BinarySearchTree, HeightConventionAndDegeneration)
{
    Payload payload {0};
    hubflow::ds::BinarySearchTree<std::string, Payload> tree;
    EXPECT_EQ(tree.height(), -1);
    tree.insert(code(1001), &payload);
    EXPECT_EQ(tree.height(), 0);
    for (int number = 1002; number <= 1004; ++number)
    {
        tree.insert(code(number), &payload);
    }
    EXPECT_EQ(tree.height(), 3);
    EXPECT_EQ(tree.find(code(1004)).visited, 4U);
}

TEST(BPlusTree, SplitsLeavesAndInternalNodesKeepingLeavesAtSameDepth)
{
    std::vector<Payload> payloads(20);
    hubflow::ds::BPlusTree<std::string, Payload, 4> tree;
    EXPECT_EQ(tree.levels(), 0U);
    for (int i = 0; i < 20; ++i)
    {
        payloads[i].value = 1001 + i;
        EXPECT_TRUE(tree.insert(code(1001 + i), &payloads[i]));
        EXPECT_TRUE(tree.leavesAtSameDepth());
    }
    EXPECT_GE(tree.levels(), 3U);
    EXPECT_FALSE(tree.root()->isLeaf);
    EXPECT_FALSE(tree.insert(code(1010), &payloads[0]));
    for (int i = 0; i < 20; ++i)
    {
        EXPECT_EQ(tree.find(code(1001 + i)).pValue, &payloads[i]);
    }
    EXPECT_EQ(tree.find("PKG-0999").pValue, nullptr);
}

TEST(BPlusTree, RootSplitsAfterFourthKey)
{
    Payload payload {0};
    hubflow::ds::BPlusTree<std::string, Payload, 4> tree;
    for (int number = 1001; number <= 1003; ++number)
    {
        tree.insert(code(number), &payload);
    }
    EXPECT_TRUE(tree.root()->isLeaf);
    tree.insert(code(1004), &payload);
    EXPECT_FALSE(tree.root()->isLeaf);
    EXPECT_EQ(tree.root()->keyCount, 1U);
    EXPECT_EQ(tree.root()->keys[0], "PKG-1003");
    EXPECT_EQ(tree.levels(), 2U);
}

TEST(BPlusTree, RangeUsesLeafLinksAndReturnsAscendingKeys)
{
    std::vector<Payload> payloads(12);
    hubflow::ds::BPlusTree<std::string, Payload, 4> tree;
    const int order[] {1008, 1003, 1011, 1001, 1006, 1004, 1012, 1002, 1009, 1005, 1010, 1007};
    for (std::size_t i = 0; i < 12; ++i)
    {
        tree.insert(code(order[i]), &payloads[i]);
    }
    std::vector<std::string> keys;
    tree.range("PKG-1003", "PKG-1006", [&keys](const std::string& key, Payload*) { keys.push_back(key); });
    EXPECT_EQ(keys, (std::vector<std::string> {"PKG-1003", "PKG-1004", "PKG-1005", "PKG-1006"}));

    keys.clear();
    tree.range("PKG-2000", "PKG-3000", [&keys](const std::string& key, Payload*) { keys.push_back(key); });
    EXPECT_TRUE(keys.empty());

    std::vector<std::string> all;
    tree.forEachInOrder([&all](const std::string& key, Payload*) { all.push_back(key); });
    EXPECT_EQ(all.size(), 12U);
    EXPECT_TRUE(std::is_sorted(all.begin(), all.end()));
}

TEST(BPlusTree, EmptyTreeQueriesAreSafe)
{
    hubflow::ds::BPlusTree<std::string, Payload, 4> tree;
    EXPECT_EQ(tree.find("PKG-1001").pValue, nullptr);
    EXPECT_EQ(tree.range("PKG-1001", "PKG-1009", [](const std::string&, Payload*) {}), 0U);
    EXPECT_TRUE(tree.leavesAtSameDepth());
}
