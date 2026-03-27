#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../headers/heaps/BinaryHeap.tpp"

using namespace aoi;

using BinaryMaxHeap = BinaryHeap<int, int, std::greater<int>>;
using BinaryMinHeap = BinaryHeap<int, int, std::less<int>>;

void test_basic_max_order() {
  std::cout << "Testing max-heap push/pop order... ";
  BinaryMaxHeap h;

  h.push(5, 100);
  h.push(8, 300);
  h.push(3, 200);

  assert(h.size() == 3);
  assert(!h.empty());

  assert(h.peek() == 300);
  h.pop();
  assert(h.peek() == 100);
  h.pop();
  assert(h.peek() == 200);
  h.pop();

  assert(h.empty());
  std::cout << "passed\n";
}

void test_basic_min_order() {
  std::cout << "Testing min-heap push/pop order... ";
  BinaryMinHeap h;

  h.push(5, 100);
  h.push(8, 300);
  h.push(3, 200);

  assert(h.peek() == 200);
  h.pop();
  assert(h.peek() == 100);
  h.pop();
  assert(h.peek() == 300);

  std::cout << "passed\n";
}

void test_peek_with_priority() {
  std::cout << "Testing peekWithPriority... ";
  BinaryMaxHeap h;

  h.push(5, 100);
  h.push(20, 400);
  h.push(10, 200);

  auto [value, priority] = h.peekWithPriority();
  assert(value == 400);
  assert(priority == 20);
  std::cout << "passed\n";
}

void test_merge() {
  std::cout << "Testing merge... ";
  BinaryMaxHeap a;
  BinaryMaxHeap b;

  a.push(10, 1000);
  a.push(30, 3000);
  b.push(40, 4000);
  b.push(5, 500);

  a.merge(b);

  assert(a.size() == 4);
  assert(b.empty());
  assert(a.peek() == 4000);

  a.pop();
  assert(a.peek() == 3000);
  a.pop();
  assert(a.peek() == 1000);
  a.pop();
  assert(a.peek() == 500);
  a.pop();
  assert(a.empty());

  std::cout << "passed\n";
}

void test_clear_and_iterators() {
  std::cout << "Testing iterators and clear... ";
  BinaryMaxHeap h;

  h.push(1, 10);
  h.push(2, 20);
  h.push(3, 30);

  size_t count = 0;
  for (const auto& node : h) {
    assert(node.value >= 10 && node.value <= 30);
    ++count;
  }
  assert(count == 3);

  h.clear();
  assert(h.empty());
  assert(h.size() == 0);

  std::cout << "passed\n";
}

void test_rvalue_push_and_exceptions() {
  std::cout << "Testing rvalue push and exceptions... ";
  BinaryHeap<std::string, int, std::greater<int>> h;

  std::string s = "movable";
  h.push(10, std::move(s));
  assert(h.peek() == "movable");

  h.pop();

  try {
    h.peek();
    assert(false && "peek() on empty heap must throw");
  } catch (const std::runtime_error&) {
  }

  try {
    h.pop();
    assert(false && "pop() on empty heap must throw");
  } catch (const std::runtime_error&) {
  }

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== BinaryHeap Test Suite ===\n\n";

  test_basic_max_order();
  test_basic_min_order();
  test_peek_with_priority();
  test_merge();
  test_clear_and_iterators();
  test_rvalue_push_and_exceptions();

  std::cout << "\n=== BinaryHeap tests passed! ===\n";
  return 0;
}
