#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../headers/heaps/Heap.tpp"

using namespace aoi;

using MaxHeap = HeapDecreasing<int, int, std::greater<int>>;
using MinHeap = HeapDecreasing<int, int, std::less<int>>;

template <typename H>
size_t findIndexByValue(const H& h, int value) {
  size_t idx = 0;
  for (const auto& item : h) {
    if (item.second == value) return idx;
    ++idx;
  }
  throw std::runtime_error("Value not found in heap data");
}

template <typename H>
void test_push_pop_order(const char* name, const std::vector<int>& expectedOrder) {
  std::cout << "Testing push/pop order (" << name << ")... ";
  H h(10);
  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);
  h.push(7, 3);

  assert(h.size() == expectedOrder.size());
  for (int expectedValue : expectedOrder) {
    assert(!h.isEmpty());
    assert(h.peek() == expectedValue);
    h.pop();
  }
  assert(h.isEmpty());
  std::cout << "passed\n";
}

template <typename H>
void test_peek_with_priority(const char* name, int expectedPriority, int expectedValue) {
  std::cout << "Testing peekWithPriority (" << name << ")... ";
  H h(8);
  h.push(42, 0);
  h.push(17, 1);
  h.push(99, 2);
  h.push(expectedPriority, expectedValue);

  const int top = h.peek();
  auto pair = h.peekWithPriority();

  assert(top == expectedValue);
  assert((pair.first == expectedValue && pair.second == expectedPriority) ||
         (pair.first == expectedPriority && pair.second == expectedValue));
  std::cout << "passed\n";
}

template <typename H>
void test_decrease_by_value(const char* name, int improvedPriority, int expectedTopValue) {
  std::cout << "Testing decreasePriorityByValue (" << name << ")... ";
  H h(10);
  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  h.decreasePriorityByValue(0, improvedPriority);
  assert(h.peek() == expectedTopValue);
  assert(h.containsValue(0));
  std::cout << "passed\n";
}

template <typename H>
void test_decrease_by_index(const char* name, int improvedPriority, int expectedTopValue) {
  std::cout << "Testing decreasePriority(index) (" << name << ")... ";
  H h(10);
  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  const size_t idx = findIndexByValue(h, 0);
  h.decreasePriority(idx, improvedPriority);
  assert(h.peek() == expectedTopValue);
  std::cout << "passed\n";
}

template <typename H>
void test_contains_delete_and_pop_updates(const char* name) {
  std::cout << "Testing contains/delete/pop updates (" << name << ")... ";
  H h(10);
  h.push(10, 0);
  h.push(30, 2);
  h.push(20, 1);

  assert(h.containsValue(0));
  assert(h.containsValue(1));
  assert(h.containsValue(2));

  h.deleteByValue(1);
  assert(!h.containsValue(1));
  assert(h.size() == 2);

  int top = h.peek();
  h.pop();
  assert(!h.containsValue(top));
  std::cout << "passed\n";
}

template <typename H>
void test_merge_and_clear(const char* name, int expectedTopAfterMerge) {
  std::cout << "Testing merge and clear (" << name << ")... ";
  H a(10);
  H b(10);

  a.push(10, 1);
  a.push(30, 3);
  b.push(20, 2);
  b.push(40, 4);

  a.merge(b);
  assert(a.size() == 4);
  assert(b.isEmpty());
  assert(a.peek() == expectedTopAfterMerge);

  for (int v = 1; v <= 4; ++v) {
    assert(a.containsValue(v));
  }

  a.clear();
  assert(a.isEmpty());
  assert(a.size() == 0);
  for (int v = 1; v <= 4; ++v) {
    assert(!a.containsValue(v));
  }
  std::cout << "passed\n";
}

template <typename H>
void test_exceptions(const char* name) {
  std::cout << "Testing exceptions (" << name << ")... ";

  H h(8);
  try {
    h.pop();
    assert(false && "pop() on empty heap must throw");
  } catch (const std::runtime_error&) {
  }

  try {
    h.peek();
    assert(false && "peek() on empty heap must throw");
  } catch (const std::runtime_error&) {
  }

  try {
    h.deleteByValue(0);
    assert(false && "deleteByValue() on empty heap must throw");
  } catch (const std::runtime_error&) {
  }

  h.push(5, 0);
  try {
    h.deleteByValue(7);
    assert(false && "deleteByValue() for absent value must throw");
  } catch (const std::runtime_error&) {
  }

  try {
    h.decreasePriority(100, 1);
    assert(false && "decreasePriority() out of range must throw");
  } catch (const std::out_of_range&) {
  }

  H noMap;
  try {
    noMap.decreasePriorityByValue(0, 10);
    assert(false && "decreasePriorityByValue() without indexMap must throw");
  } catch (const std::runtime_error&) {
  }

  std::cout << "passed\n";
}

template <typename H>
void run_suite(const char* name, const std::vector<int>& expectedOrder, int improvedPriority,
               int expectedTopAfterImprove, int expectedPriorityForPeek, int expectedTopAfterMerge) {
  test_push_pop_order<H>(name, expectedOrder);
  test_peek_with_priority<H>(name, expectedPriorityForPeek, 5);
  test_decrease_by_value<H>(name, improvedPriority, expectedTopAfterImprove);
  test_decrease_by_index<H>(name, improvedPriority, expectedTopAfterImprove);
  test_contains_delete_and_pop_updates<H>(name);
  test_merge_and_clear<H>(name, expectedTopAfterMerge);
  test_exceptions<H>(name);
}

int main() {
  std::cout << "=== HeapDecreasing Test Suite ===\n\n";

  run_suite<MaxHeap>("HeapDecreasing/max", {2, 3, 0, 1}, 20, 0, 120, 4);
  run_suite<MinHeap>("HeapDecreasing/min", {1, 0, 3, 2}, 1, 0, 1, 1);

  std::cout << "\n=== HeapDecreasing tests passed! ===\n";
  return 0;
}
