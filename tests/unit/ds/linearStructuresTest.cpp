#include "datastructures/doublyLinkedList.hpp"
#include "datastructures/dynamicArray.hpp"
#include "datastructures/queue.hpp"
#include "datastructures/singlyLinkedList.hpp"
#include "datastructures/stack.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace
{
    template<typename TContainer>
    std::vector<int> collect(const TContainer& container)
    {
        std::vector<int> values;
        container.forEach([&values](const int value) { values.push_back(value); });
        return values;
    }
} // namespace

TEST(SinglyLinkedList, PushesAtBothEndsAndPopsFromFront)
{
    hubflow::ds::SinglyLinkedList<int> list;
    list.pushBack(2);
    list.pushFront(1);
    list.pushBack(3);

    int first {0};
    EXPECT_TRUE(list.popFront(first));
    EXPECT_EQ(first, 1);
    EXPECT_EQ(collect(list), (std::vector<int> {2, 3}));
    EXPECT_EQ(list.size(), 2U);
}

TEST(SinglyLinkedList, RemovesHeadMiddleAndTailKeepingTailValid)
{
    hubflow::ds::SinglyLinkedList<int> list;
    for (int value : {1, 2, 3, 4})
    {
        list.pushBack(value);
    }
    EXPECT_TRUE(list.removeFirstIf([](int value) { return value == 4; }));
    list.pushBack(5);
    EXPECT_TRUE(list.removeFirstIf([](int value) { return value == 1; }));
    EXPECT_TRUE(list.removeFirstIf([](int value) { return value == 3; }));
    EXPECT_FALSE(list.removeFirstIf([](int value) { return value == 42; }));
    EXPECT_EQ(collect(list), (std::vector<int> {2, 5}));
}

TEST(SinglyLinkedList, PopOnEmptyListReturnsFalse)
{
    hubflow::ds::SinglyLinkedList<int> list;
    int value {7};
    EXPECT_FALSE(list.popFront(value));
    EXPECT_EQ(value, 7);
    EXPECT_TRUE(list.isEmpty());
}

TEST(DoublyLinkedList, TraversesForwardAndBackwardWithPrevLinks)
{
    hubflow::ds::DoublyLinkedList<int> list;
    for (int value : {1, 2, 3})
    {
        list.pushBack(value);
    }
    std::vector<int> reversed;
    list.forEachReverse([&reversed](const int value) { reversed.push_back(value); });

    EXPECT_EQ(collect(list), (std::vector<int> {1, 2, 3}));
    EXPECT_EQ(reversed, (std::vector<int> {3, 2, 1}));
    EXPECT_EQ(list.tail()->pPrev->value, 2);
}

TEST(DoublyLinkedList, SingleNodeIsHeadAndTail)
{
    hubflow::ds::DoublyLinkedList<int> list;
    list.pushBack(9);
    EXPECT_EQ(list.head(), list.tail());
    EXPECT_EQ(list.head()->pPrev, nullptr);
    EXPECT_EQ(list.head()->pNext, nullptr);
}

TEST(Queue, IsFifoAndUpdatesFrontAndRear)
{
    hubflow::ds::Queue<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    EXPECT_EQ(queue.front(), 1);
    EXPECT_EQ(queue.rear(), 3);
    EXPECT_EQ(queue.dequeue(), 1);
    EXPECT_EQ(queue.dequeue(), 2);
    EXPECT_EQ(queue.front(), 3);
    EXPECT_EQ(queue.rear(), 3);
    EXPECT_EQ(queue.dequeue(), 3);
    EXPECT_TRUE(queue.isEmpty());
    queue.enqueue(4);
    EXPECT_EQ(queue.front(), 4);
    EXPECT_EQ(queue.rear(), 4);
}

TEST(Queue, ThrowsOnEmptyAccess)
{
    hubflow::ds::Queue<int> queue;
    EXPECT_THROW(queue.dequeue(), std::underflow_error);
    EXPECT_THROW(static_cast<void>(queue.front()), std::underflow_error);
}

TEST(Stack, IsLifo)
{
    hubflow::ds::Stack<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    EXPECT_EQ(stack.top(), 3);
    EXPECT_EQ(collect(stack), (std::vector<int> {3, 2, 1}));
    EXPECT_EQ(stack.pop(), 3);
    EXPECT_EQ(stack.pop(), 2);
    EXPECT_EQ(stack.pop(), 1);
    EXPECT_TRUE(stack.isEmpty());
    EXPECT_THROW(stack.pop(), std::underflow_error);
    EXPECT_THROW(static_cast<void>(stack.top()), std::underflow_error);
}

TEST(DynamicArray, GrowsCopiesAndValidatesIndexes)
{
    hubflow::ds::DynamicArray<int> array;
    for (int i = 0; i < 10; ++i)
    {
        array.pushBack(i);
    }
    hubflow::ds::DynamicArray<int> copy {array};
    copy[0] = 99;

    EXPECT_EQ(array.size(), 10U);
    EXPECT_EQ(array[0], 0);
    EXPECT_EQ(copy[0], 99);
    EXPECT_THROW(static_cast<void>(array[10]), std::out_of_range);
    array.popBack();
    EXPECT_EQ(array.size(), 9U);
}
