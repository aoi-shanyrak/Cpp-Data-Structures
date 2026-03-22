#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../headers/Heap.tpp"

using namespace aoi;

using MaxHeapDec = HeapDecreasing<int, int, std::greater<int>>;
using MinHeapDec = HeapDecreasing<int, int, std::less<int>>;

void test_constructor_with_maxsize() {
  std::cout << "Testing constructor with maxSize... ";
  MaxHeapDec h(10);
  assert(h.isEmpty());
  assert(h.size() == 0);
  std::cout << "passed\n";
}

void test_basic_operations() {
  std::cout << "Testing basic operations... ";
  MaxHeapDec h(10);

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
  MaxHeapDec h(10);

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

void test_decrease_priority_by_index() {
  std::cout << "Testing decreasePriority by index... ";
  MaxHeapDec h(10);

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

  h.decreasePriority(idx, 10);

  // Now value 0 should be on top with priority 10
  assert(h.peek() == 0);

  std::cout << "passed\n";
}

void test_increase_priority_by_value() {
  std::cout << "Testing decreasePriorityByValue... ";
  MaxHeapDec h(10);

  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  // Initially: priority 8 (value 2) is on top
  assert(h.peek() == 2);

  // Increase priority of value 1 to 15
  h.decreasePriorityByValue(1, 15);

  // Now value 1 should be on top with priority 15
  assert(h.peek() == 1);

  // Increase priority of value 0 to 20
  h.decreasePriorityByValue(0, 20);

  // Now value 0 should be on top with priority 20
  assert(h.peek() == 0);

  std::cout << "passed\n";
}

void test_increase_priority_maintains_heap_property() {
  std::cout << "Testing that decreasePriority maintains heap property... ";
  MaxHeapDec h(20);

  // Build a larger heap
  for (int i = 0; i < 10; ++i) {
    h.push(i, i);
  }

  // Increase priority of value 0 (lowest) to highest
  h.decreasePriorityByValue(0, 100);

  // Should now be on top
  assert(h.peek() == 0);

  // Pop and verify the rest are in order
  h.pop();
  assert(h.peek() == 9);  // priority 9 is next highest

  std::cout << "passed\n";
}

void test_decrease_priority_exception_out_of_range() {
  std::cout << "Testing decreasePriority exception (out of range)... ";
  MaxHeapDec h(10);

  h.push(5, 0);
  h.push(3, 1);

  try {
    h.decreasePriority(10, 100);  // Index 10 doesn't exist
    assert(false && "Should throw exception");
  } catch (const std::out_of_range& e) {
    // Expected
  }

  std::cout << "passed\n";
}

void test_decrease_priority_by_value_exception_not_in_heap() {
  std::cout << "Testing decreasePriorityByValue exception (not in heap)... ";
  MaxHeapDec h(10);

  h.push(5, 0);
  h.push(3, 1);

  try {
    h.decreasePriorityByValue(5, 100);  // Value 5 not in heap
    assert(false && "Should throw exception");
  } catch (const std::runtime_error& e) {
    // Expected
  }

  std::cout << "passed\n";
}

void test_pop_updates_index_map() {
  std::cout << "Testing that pop updates indexMap... ";
  MaxHeapDec h(10);

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
  std::cout << "Testing multiple decrease operations... ";
  MaxHeapDec h(15);

  for (int i = 0; i < 10; ++i) {
    h.push(i, i);
  }

  // Increase several priorities
  h.decreasePriorityByValue(0, 50);
  h.decreasePriorityByValue(3, 45);
  h.decreasePriorityByValue(7, 40);

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

void test_min_heap_with_decrease() {
  std::cout << "Testing min heap with decreasePriority... ";
  // Min-heap: lower priority comes first
  MinHeapDec h(10);

  h.push(5, 0);
  h.push(3, 1);
  h.push(8, 2);

  // Min heap: priority 3 (value 1) should be on top
  assert(h.peek() == 1);

  // For min-heap, improving priority means decreasing numeric value.
  h.decreasePriorityByValue(0, 1);

  // Value 0 should move to top (priority 1 is smallest)
  assert(h.peek() == 0);

  std::cout << "passed\n";
}

void test_copy_constructor() {
  std::cout << "Testing copy constructor... ";
  MaxHeapDec h1(10);
  h1.push(5, 0);
  h1.push(3, 1);
  h1.push(8, 2);

  MaxHeapDec h2(h1);

  assert(h2.size() == h1.size());
  assert(h2.peek() == h1.peek());
  assert(h2.containsValue(0));
  assert(h2.containsValue(1));
  assert(h2.containsValue(2));

  // Modify h2
  h2.decreasePriorityByValue(0, 100);
  assert(h2.peek() == 0);

  // h1 should be unchanged
  assert(h1.peek() == 2);

  std::cout << "passed\n";
}

void test_move_constructor() {
  std::cout << "Testing move constructor... ";
  MaxHeapDec h1(10);
  h1.push(5, 0);
  h1.push(3, 1);
  h1.push(8, 2);

  size_t old_size = h1.size();
  MaxHeapDec h2(std::move(h1));

  assert(h2.size() == old_size);
  assert(h2.peek() == 2);
  assert(h2.containsValue(0));
  assert(h2.containsValue(1));
  assert(h2.containsValue(2));

  std::cout << "passed\n";
}

void test_clear() {
  std::cout << "Testing clear... ";
  MaxHeapDec h(10);
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
  MaxHeapDec h(10);

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
  MaxHeapDec h(10);

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
  MinHeapDec pq(6);  // 6 vertices, min-heap

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
  pq.decreasePriorityByValue(1, 4);
  // Edge 0->2 with weight 2
  pq.decreasePriorityByValue(2, 2);

  // Next vertex should be 2 (distance 2) - smallest distance
  assert(pq.peek() == 2);

  auto [dist2, v2] = pq.popWithPriority();
  assert(v2 == 2 && dist2 == 2);

  // Update through vertex 2
  // Edge 2->3 with weight 3 (total: 2+3=5)
  pq.decreasePriorityByValue(3, 5);

  // Next should be vertex 1 (distance 4) - next smallest
  assert(pq.peek() == 1);

  std::cout << "passed\n";
}

void test_merge_preserves_index_map() {
  std::cout << "Testing merge preserves index map... ";
  MaxHeapDec h1(10);
  MaxHeapDec h2(10);

  h1.push(10, 1);
  h1.push(30, 3);
  h2.push(20, 2);
  h2.push(40, 4);

  h1.merge(h2);

  assert(h1.size() == 4);
  assert(h2.isEmpty());

  assert(h1.containsValue(1));
  assert(h1.containsValue(2));
  assert(h1.containsValue(3));
  assert(h1.containsValue(4));

  assert(h1.peek() == 4);  // priority 40
  h1.pop();
  assert(h1.peek() == 3);  // priority 30

  // If indexMap is valid, this should reposition value 1 to the top.
  h1.decreasePriorityByValue(1, 100);
  assert(h1.peek() == 1);

  std::cout << "passed\n";
}

void test_merge_with_empty_heap() {
  std::cout << "Testing merge with empty heap... ";
  MaxHeapDec h1(10);
  MaxHeapDec h2(10);

  h1.push(5, 0);
  h1.push(8, 2);

  h1.merge(h2);
  assert(h1.size() == 2);
  assert(h2.isEmpty());
  assert(h1.containsValue(0));
  assert(h1.containsValue(2));

  MaxHeapDec h3(10);
  h3.merge(h1);
  assert(h1.isEmpty());
  assert(h3.size() == 2);
  assert(h3.containsValue(0));
  assert(h3.containsValue(2));
  assert(h3.peek() == 2);

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== HeapWithInc Tests ===\n\n";

  test_constructor_with_maxsize();
  test_basic_operations();
  test_contains_value();
  test_decrease_priority_by_index();
  test_increase_priority_by_value();
  test_increase_priority_maintains_heap_property();
  test_decrease_priority_exception_out_of_range();
  test_decrease_priority_by_value_exception_not_in_heap();
  test_pop_updates_index_map();
  test_multiple_increase_operations();
  test_min_heap_with_decrease();
  test_copy_constructor();
  test_move_constructor();
  test_clear();
  test_peek_with_priority();
  test_pop_with_priority();
  test_dijkstra_like_scenario();
  test_merge_preserves_index_map();
  test_merge_with_empty_heap();

  std::cout << "\n=== All HeapWithInc tests passed! ===\n";
  return 0;
}
