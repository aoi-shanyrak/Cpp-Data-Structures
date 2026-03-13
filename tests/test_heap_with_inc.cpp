#include "../headers/HeapWithInc.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

void test_constructor_with_maxsize() {
  std::cout << "Testing constructor with maxSize... ";
  HeapWithInc<int> h(10);
  assert(h.isEmpty());
  assert(h.size() == 0);
  std::cout << "passed\n";
}

void test_basic_operations() {
  std::cout << "Testing basic operations... ";
  HeapWithInc<int> h(10);

  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  assert(h.size() == 3);
  assert(!h.isEmpty());
  assert(h.peek() == 2);  // priority 8 is highest

  h.pop();
  assert(h.peek() == 0);  // priority 5 is next

  h.pop();
  assert(h.peek() == 1);  // priority 3 is last
  std::cout << "passed\n";
}

void test_contains_value() {
  std::cout << "Testing containsValue... ";
  HeapWithInc<int> h(10);

  h.push(5, 0);
  h.push(3, 2);
  h.push(8, 5);

  assert(h.containsValue(0));
  assert(h.containsValue(2));
  assert(h.containsValue(5));
  assert(!h.containsValue(1));
  assert(!h.containsValue(3));
  assert(!h.containsValue(4));

  h.pop();  // Remove value 5
  assert(!h.containsValue(5));
  assert(h.containsValue(0));
  assert(h.containsValue(2));

  std::cout << "passed\n";
}

void test_increase_priority_by_index() {
  std::cout << "Testing increasePriority by index... ";
  HeapWithInc<int> h(10);

  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  // Initially: priority 8 (value 2) is on top
  assert(h.peek() == 2);

  // Find which index has value 0 and increase its priority to 10
  size_t idx = 0;
  for (auto& pair : h) {
    if (pair.second == 0) break;
    idx++;
  }

  h.increasePriority(idx, 10);

  // Now value 0 should be on top with priority 10
  assert(h.peek() == 0);

  std::cout << "passed\n";
}

void test_increase_priority_by_value() {
  std::cout << "Testing increasePriorityByValue... ";
  HeapWithInc<int> h(10);

  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  // Initially: priority 8 (value 2) is on top
  assert(h.peek() == 2);

  // Increase priority of value 1 to 15
  h.increasePriorityByValue(1, 15);

  // Now value 1 should be on top with priority 15
  assert(h.peek() == 1);

  // Increase priority of value 0 to 20
  h.increasePriorityByValue(0, 20);

  // Now value 0 should be on top with priority 20
  assert(h.peek() == 0);

  std::cout << "passed\n";
}

void test_increase_priority_maintains_heap_property() {
  std::cout << "Testing that increasePriority maintains heap property... ";
  HeapWithInc<int> h(20);

  // Build a larger heap
  for (int i = 0; i < 10; ++i) {
    h.push(i, i);
  }

  // Increase priority of value 0 (lowest) to highest
  h.increasePriorityByValue(0, 100);

  // Should now be on top
  assert(h.peek() == 0);

  // Pop and verify the rest are in order
  h.pop();
  assert(h.peek() == 9);  // priority 9 is next highest

  std::cout << "passed\n";
}

void test_increase_priority_exception_out_of_range() {
  std::cout << "Testing increasePriority exception (out of range)... ";
  HeapWithInc<int> h(10);

  h.push(5, 0);
  h.push(3, 1);

  try {
    h.increasePriority(10, 100);  // Index 10 doesn't exist
    assert(false && "Should throw exception");
  } catch (const std::out_of_range& e) {
    // Expected
  }

  std::cout << "passed\n";
}

void test_increase_priority_by_value_exception_not_in_heap() {
  std::cout << "Testing increasePriorityByValue exception (not in heap)... ";
  HeapWithInc<int> h(10);

  h.push(5, 0);
  h.push(3, 1);

  try {
    h.increasePriorityByValue(5, 100);  // Value 5 not in heap
    assert(false && "Should throw exception");
  } catch (const std::runtime_error& e) {
    // Expected
  }

  std::cout << "passed\n";
}

void test_pop_updates_index_map() {
  std::cout << "Testing that pop updates indexMap... ";
  HeapWithInc<int> h(10);

  h.push(10, 0);
  h.push(20, 1);
  h.push(15, 2);

  assert(h.containsValue(1));
  assert(h.peek() == 1);  // priority 20 is highest

  h.pop();

  // Value 1 should no longer be in heap
  assert(!h.containsValue(1));
  assert(h.containsValue(0));
  assert(h.containsValue(2));

  std::cout << "passed\n";
}

void test_multiple_increase_operations() {
  std::cout << "Testing multiple increase operations... ";
  HeapWithInc<int> h(15);

  for (int i = 0; i < 10; ++i) {
    h.push(i, i);
  }

  // Increase several priorities
  h.increasePriorityByValue(0, 50);
  h.increasePriorityByValue(3, 45);
  h.increasePriorityByValue(7, 40);

  // Check order
  assert(h.peek() == 0);
  h.pop();
  assert(h.peek() == 3);
  h.pop();
  assert(h.peek() == 7);
  h.pop();
  assert(h.peek() == 9);  // Next highest original priority

  std::cout << "passed\n";
}

void test_min_heap_with_increase() {
  std::cout << "Testing min heap with increasePriority... ";
  // Min-heap: lower priority comes first
  HeapWithInc<int, int, std::less<int>> h(10);

  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  // Min heap: priority 3 (value 1) should be on top
  assert(h.peek() == 1);

  // Note: in min-heap, "increase" actually means making priority smaller
  // To make value 2 come first, we'd need to decrease its priority
  // But the function is called "increasePriority" so it increases the numeric value

  // Increase priority of value 0 to 10
  h.increasePriorityByValue(0, 10);

  // In min-heap, this pushes it down, so value 1 (priority 3) is still on top
  assert(h.peek() == 1);

  std::cout << "passed\n";
}

void test_copy_constructor() {
  std::cout << "Testing copy constructor... ";
  HeapWithInc<int> h1(10);
  h1.push(5, 0);
  h1.push(3, 1);
  h1.push(8, 2);

  HeapWithInc<int> h2(h1);

  assert(h2.size() == h1.size());
  assert(h2.peek() == h1.peek());
  assert(h2.containsValue(0));
  assert(h2.containsValue(1));
  assert(h2.containsValue(2));

  // Modify h2
  h2.increasePriorityByValue(0, 100);
  assert(h2.peek() == 0);

  // h1 should be unchanged
  assert(h1.peek() == 2);

  std::cout << "passed\n";
}

void test_move_constructor() {
  std::cout << "Testing move constructor... ";
  HeapWithInc<int> h1(10);
  h1.push(5, 0);
  h1.push(3, 1);
  h1.push(8, 2);

  size_t old_size = h1.size();
  HeapWithInc<int> h2(std::move(h1));

  assert(h2.size() == old_size);
  assert(h2.peek() == 2);
  assert(h2.containsValue(0));
  assert(h2.containsValue(1));
  assert(h2.containsValue(2));

  std::cout << "passed\n";
}

void test_clear() {
  std::cout << "Testing clear... ";
  HeapWithInc<int> h(10);
  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  assert(!h.isEmpty());
  h.clear();
  assert(h.isEmpty());
  assert(h.size() == 0);

  std::cout << "passed\n";
}

void test_peek_with_priority() {
  std::cout << "Testing peekWithPriority... ";
  HeapWithInc<int> h(10);

  h.push(42, 0);
  h.push(17, 1);
  h.push(99, 2);

  auto [priority, value] = h.peekWithPriority();
  assert(priority == 99);
  assert(value == 2);

  h.pop();

  auto [p2, v2] = h.peekWithPriority();
  assert(p2 == 42);
  assert(v2 == 0);

  std::cout << "passed\n";
}

void test_pop_with_priority() {
  std::cout << "Testing popWithPriority... ";
  HeapWithInc<int> h(10);

  h.push(42, 0);
  h.push(17, 1);
  h.push(99, 2);

  auto [p1, v1] = h.popWithPriority();
  assert(p1 == 99 && v1 == 2);
  assert(h.size() == 2);

  auto [p2, v2] = h.popWithPriority();
  assert(p2 == 42 && v2 == 0);
  assert(h.size() == 1);

  auto [p3, v3] = h.popWithPriority();
  assert(p3 == 17 && v3 == 1);
  assert(h.isEmpty());

  std::cout << "passed\n";
}

void test_dijkstra_like_scenario() {
  std::cout << "Testing Dijkstra-like scenario... ";
  // Simulate Dijkstra's algorithm usage with MIN heap (smaller distances first)
  HeapWithInc<int, int, std::less<int>> pq(6);  // 6 vertices, min-heap

  // Initial distances (vertex 0 starts at 0, others at "infinity" = 1000)
  pq.push(0, 0);  // vertex 0, distance 0
  pq.push(1000, 1);  // vertex 1, distance 1000
  pq.push(1000, 2);  // vertex 2, distance 1000
  pq.push(1000, 3);  // vertex 3, distance 1000

  // Process vertex 0 (distance 0) - should be first in min-heap
  auto [dist0, v0] = pq.popWithPriority();
  assert(v0 == 0 && dist0 == 0);

  // Update distances through vertex 0
  // Note: in min-heap, we need to set lower numeric values
  // Edge 0->1 with weight 4
  pq.increasePriorityByValue(1, 4);
  // Edge 0->2 with weight 2
  pq.increasePriorityByValue(2, 2);

  // Next vertex should be 2 (distance 2) - smallest distance
  assert(pq.peek() == 2);

  auto [dist2, v2] = pq.popWithPriority();
  assert(v2 == 2 && dist2 == 2);

  // Update through vertex 2
  // Edge 2->3 with weight 3 (total: 2+3=5)
  pq.increasePriorityByValue(3, 5);

  // Next should be vertex 1 (distance 4) - next smallest
  assert(pq.peek() == 1);

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== HeapWithInc Tests ===\n\n";

  test_constructor_with_maxsize();
  test_basic_operations();
  test_contains_value();
  test_increase_priority_by_index();
  test_increase_priority_by_value();
  test_increase_priority_maintains_heap_property();
  test_increase_priority_exception_out_of_range();
  test_increase_priority_by_value_exception_not_in_heap();
  test_pop_updates_index_map();
  test_multiple_increase_operations();
  test_min_heap_with_increase();
  test_copy_constructor();
  test_move_constructor();
  test_clear();
  test_peek_with_priority();
  test_pop_with_priority();
  test_dijkstra_like_scenario();

  std::cout << "\n=== All HeapWithInc tests passed! ===\n";
  return 0;
}
