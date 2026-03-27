#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../headers/heaps/BinaryHeap.tpp"

using namespace aoi;

using MaxHeap = BinaryHeap<int, int, std::greater<int>>;
using MinHeap = BinaryHeap<int, int, std::less<int>>;

template <typename H>
void test_push_pop_order(const char* name, const std::vector<int>& expectedOrder) {
  std::cout << "Testing push/pop order (" << name << ")... ";
  H h;
  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);
  h.push(7, 3);

  assert(h.size() == expectedOrder.size());
  for (int expectedValue : expectedOrder) {
    assert(!h.empty());
    assert(h.peek() == expectedValue);
    h.pop();
  }
  assert(h.empty());
  std::cout << "passed\n";
}

template <typename H>
void test_peek_with_priority(const char* name, int expectedPriority, int expectedValue) {
  std::cout << "Testing peekWithPriority (" << name << ")... ";
  H h;
  h.push(42, 0);
  h.push(17, 1);
  h.push(99, 2);
  h.push(expectedPriority, expectedValue);

  auto [value, priority] = h.peekWithPriority();

  assert(value == h.peek());
  assert(value == expectedValue);
  assert(priority == expectedPriority);
  std::cout << "passed\n";
}

template <typename H>
void test_decrease_by_handle(const char* name, int improvedPriority, int expectedTopValue) {
  std::cout << "Testing decreasePriority(handle) (" << name << ")... ";
  H h;
  auto n0 = h.push(5, 0);
  auto n1 = h.push(3, 1);
  auto n2 = h.push(8, 2);

  (void)n1;
  (void)n2;

  h.decreasePriority(n0, improvedPriority);
  assert(h.peek() == expectedTopValue);
  std::cout << "passed\n";
}

template <typename H>
void test_delete_key(const char* name, int expectedTopAfterDelete) {
  std::cout << "Testing delete_key(handle) (" << name << ")... ";
  H h;
  auto n0 = h.push(10, 0);
  auto n1 = h.push(30, 1);
  auto n2 = h.push(20, 2);

  (void)n0;
  (void)n2;

  h.delete_key(n1);
  assert(h.size() == 2);
  assert(h.peek() == expectedTopAfterDelete);
  std::cout << "passed\n";
}

template <typename H>
void test_merge_and_clear(const char* name, int expectedTopAfterMerge) {
  std::cout << "Testing merge and clear (" << name << ")... ";
  H a;
  H b;

  a.push(10, 1);
  a.push(30, 3);
  b.push(20, 2);
  b.push(40, 4);

  a.merge(b);
  assert(a.size() == 4);
  assert(b.empty());
  assert(a.peek() == expectedTopAfterMerge);

  a.clear();
  assert(a.empty());
  assert(a.size() == 0);
  std::cout << "passed\n";
}

template <typename H>
void test_merge_preserves_other_handles(const char* name, int improvedPriority, int expectedTopAfterImproveFromOther) {
  std::cout << "Testing merge handle stability (" << name << ")... ";
  H a;
  H b;

  a.push(10, 10);
  auto fromOther1 = b.push(30, 30);
  auto fromOther2 = b.push(20, 20);

  a.merge(b);
  assert(b.empty());

  // Handle returned by 'b' must still refer to the same node after merge into 'a'.
  a.decreasePriority(fromOther2, improvedPriority);
  assert(a.peek() == expectedTopAfterImproveFromOther);

  a.delete_key(fromOther1);
  assert(a.size() == 2);
  std::cout << "passed\n";
}

template <typename H>
void test_exceptions(const char* name) {
  std::cout << "Testing exceptions (" << name << ")... ";

  H h;
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
    size_t fake = 123;
    h.delete_key(&fake);
    assert(false && "delete_key() for absent handle must throw");
  } catch (const std::out_of_range&) {
  }

  auto n0 = h.push(5, 0);
  (void)n0;
  try {
    size_t fake = 42;
    h.delete_key(&fake);
    assert(false && "delete_key() for invalid handle must throw");
  } catch (const std::out_of_range&) {
  }

  try {
    size_t fake = 42;
    h.decreasePriority(&fake, 1);
    assert(false && "decreasePriority() out of range must throw");
  } catch (const std::out_of_range&) {
  }

  std::cout << "passed\n";
}

template <typename H>
void run_suite(const char* name, const std::vector<int>& expectedOrder, int improvedPriority,
               int expectedTopAfterImprove, int expectedPriorityForPeek, int expectedTopAfterDelete,
               int expectedTopAfterMerge, int mergeImprovedPriority, int expectedTopAfterImproveFromOther) {
  test_push_pop_order<H>(name, expectedOrder);
  test_peek_with_priority<H>(name, expectedPriorityForPeek, 5);
  test_decrease_by_handle<H>(name, improvedPriority, expectedTopAfterImprove);
  test_delete_key<H>(name, expectedTopAfterDelete);
  test_merge_and_clear<H>(name, expectedTopAfterMerge);
  test_merge_preserves_other_handles<H>(name, mergeImprovedPriority, expectedTopAfterImproveFromOther);
  test_exceptions<H>(name);
}

int main() {
  std::cout << "=== BinaryHeap Handle API Test Suite ===\n\n";

  run_suite<MaxHeap>("BinaryHeap/max", {2, 3, 0, 1}, 20, 0, 120, 2, 4, 40, 20);
  run_suite<MinHeap>("BinaryHeap/min", {1, 0, 3, 2}, 1, 0, 1, 0, 1, 1, 20);

  std::cout << "\n=== BinaryHeap handle tests passed! ===\n";
  return 0;
}
