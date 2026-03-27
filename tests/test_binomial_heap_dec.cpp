#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../headers/heaps/BinomialHeap.tpp"

using namespace aoi;

using BinomialMaxHeap = BinomialHeap<int, int, std::greater<int>>;
using BinomialMinHeap = BinomialHeap<int, int, std::less<int>>;

template <typename H>
void test_push_pop_order(const char* name, const std::vector<int>& expectedOrder) {
  std::cout << "Testing push/pop order (" << name << ")...\n";
  H h;
  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

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
  std::cout << "Testing peekWithPriority (" << name << ")...\n";
  H h;
  h.push(42, 0);
  h.push(17, 1);
  h.push(99, 2);
  h.push(expectedPriority, expectedValue);

  auto [value, priority] = h.peekWithPriority();
  assert(value == h.peek());
  assert(priority == expectedPriority);
  assert(value == expectedValue);
  std::cout << "passed\n";
}

template <typename H>
void test_handle_decrease(const char* name, int improvedPriority, int expectedTop) {
  std::cout << "Testing handle decrease (" << name << ")...\n";
  H h;
  auto n0 = h.push(5, 0);
  auto n1 = h.push(3, 1);
  auto n2 = h.push(8, 2);

  (void)n1;
  (void)n2;

  h.decreasePriority(n0, improvedPriority);
  assert(h.peek() == expectedTop);

  assert(!h.empty());
  std::cout << "passed\n";
}

template <typename H>
void test_merge(const char* name, int expectedTop) {
  std::cout << "Testing merge (" << name << ")...\n";
  H a;
  H b;

  a.push(10, 1);
  b.push(20, 2);
  b.push(30, 3);

  a.merge(b);
  assert(b.empty());
  assert(!a.empty());
  assert(a.peek() == expectedTop);

  int count = 0;
  while (!a.empty()) {
    a.pop();
    ++count;
  }
  assert(count == 3);
  std::cout << "passed\n";
}

template <typename H>
void test_exceptions(const char* name) {
  std::cout << "Testing exceptions (" << name << ")...\n";
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

  std::cout << "passed\n";
}

template <typename H>
void run_suite(const char* name, const std::vector<int>& expectedOrder, int improvedPriority,
               int expectedTopAfterImprove, int expectedPriorityForPeek, int expectedTopAfterMerge) {
  test_push_pop_order<H>(name, expectedOrder);
  test_peek_with_priority<H>(name, expectedPriorityForPeek, 5);
  test_handle_decrease<H>(name, improvedPriority, expectedTopAfterImprove);
  test_merge<H>(name, expectedTopAfterMerge);
  test_exceptions<H>(name);
}

int main() {
  std::cout << std::unitbuf;
  std::cout << "=== BinomialHeap Test Suite ===\n\n";

  run_suite<BinomialMaxHeap>("BinomialHeap/max", {2, 0, 1}, 20, 0, 120, 3);
  run_suite<BinomialMinHeap>("BinomialHeap/min", {1, 0, 2}, 1, 0, 1, 1);

  std::cout << "\n=== BinomialHeap tests passed! ===\n";
  return 0;
}
