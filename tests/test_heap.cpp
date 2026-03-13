#include "../headers/Heap.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

void test_constructor() {
  std::cout << "Testing constructor... ";
  Heap<int> h;
  assert(h.isEmpty());
  assert(h.size() == 0);
  std::cout << "passed\n";
}

void test_max_heap_basic() {
  std::cout << "Testing max heap basic operations... ";
  Heap<int> h;  // max-heap (std::greater)

  h.push(5, 100);
  h.push(3, 200);
  h.push(8, 300);

  assert(h.size() == 3);
  assert(!h.isEmpty());

  // max priority = 8
  assert(h.peek() == 300);
  h.pop();

  // next = 5
  assert(h.peek() == 100);
  h.pop();

  // last = 3
  assert(h.peek() == 200);
  h.pop();

  assert(h.isEmpty());
  std::cout << "passed\n";
}

void test_min_heap() {
  std::cout << "Testing min heap... ";

  Heap<int, int, std::less<int>> h;  // min-heap (std::less)

  h.push(5, 100);
  h.push(3, 200);
  h.push(8, 300);

  // smallest priority = 3
  assert(h.peek() == 200);
  h.pop();

  // next = 5
  assert(h.peek() == 100);
  h.pop();

  // last = 8
  assert(h.peek() == 300);
  std::cout << "passed\n";
}

void test_same_priorities() {
  std::cout << "Testing same priorities... ";
  Heap<int> h;

  h.push(5, 1);
  h.push(5, 2);
  h.push(5, 3);

  assert(h.size() == 3);

  h.pop();
  h.pop();
  h.pop();

  assert(h.isEmpty());
  std::cout << "passed\n";
}

void test_many_elements() {
  std::cout << "Testing with many elements... ";
  Heap<int> h;

  std::vector<int> priorities = {10, 5, 20, 1, 15, 30, 25, 3, 7};
  for (size_t i = 0; i < priorities.size(); ++i) {
    h.push(priorities[i], i * 100);
  }

  assert(h.size() == priorities.size());

  while (!h.isEmpty()) {
    h.pop();
  }

  assert(h.isEmpty());
  std::cout << "passed\n";
}

void test_copy_constructor() {
  std::cout << "Testing copy constructor... ";
  Heap<int> h1;
  h1.push(5, 100);
  h1.push(3, 200);
  h1.push(8, 300);

  Heap<int> h2(h1);

  assert(h2.size() == h1.size());
  assert(h2.peek() == h1.peek());

  h2.pop();
  assert(h2.size() == 2);
  assert(h1.size() == 3);
  std::cout << "passed\n";
}

void test_move_constructor() {
  std::cout << "Testing move constructor... ";
  Heap<int> h1;
  h1.push(5, 100);
  h1.push(3, 200);
  h1.push(8, 300);

  size_t old_size = h1.size();
  Heap<int> h2(std::move(h1));

  assert(h2.size() == old_size);
  assert(h2.peek() == 300);
  std::cout << "passed\n";
}

void test_copy_assignment() {
  std::cout << "Testing copy assignment... ";
  Heap<int> h1;
  h1.push(8, 800);

  Heap<int> h2;
  h2.push(1, 100);
  h2.push(2, 200);

  h2 = h1;
  assert(h2.size() == 1);
  assert(h2.peek() == 800);
  std::cout << "passed\n";
}

void test_move_assignment() {
  std::cout << "Testing move assignment... ";
  Heap<int> h1;
  h1.push(8, 800);
  h1.push(9, 900);

  Heap<int> h2;
  h2.push(1, 100);

  h2 = std::move(h1);
  assert(h2.size() == 2);
  assert(h2.peek() == 900);
  std::cout << "passed\n";
}

void test_clear() {
  std::cout << "Testing clear... ";
  Heap<int> h;
  h.push(5, 100);
  h.push(3, 200);
  h.push(8, 300);

  assert(!h.isEmpty());
  h.clear();
  assert(h.isEmpty());
  assert(h.size() == 0);
  std::cout << "passed\n";
}

void test_with_strings() {
  std::cout << "Testing with std::string... ";
  Heap<std::string> h;

  h.push(10, "high");
  h.push(5, "medium");
  h.push(1, "low");

  assert(h.peek() == "high");
  h.pop();
  assert(h.peek() == "medium");
  h.pop();
  assert(h.peek() == "low");
  std::cout << "passed\n";
}

void test_exceptions() {
  std::cout << "Testing exceptions... ";
  Heap<int> h;

  try {
    h.pop();
    assert(false && "Should throw exception");
  } catch (const std::runtime_error& e) {
  }

  try {
    h.peek();
    assert(false && "Should throw exception");
  } catch (const std::runtime_error& e) {
  }

  std::cout << "passed\n";
}

void test_iterators() {
  std::cout << "Testing iterators... ";
  Heap<int> h;

  h.push(5, 100);
  h.push(3, 200);
  h.push(8, 300);

  size_t count = 0;
  for (auto it = h.begin(); it != h.end(); ++it) {
    count++;
  }
  assert(count == 3);
  std::cout << "passed\n";
}

void test_range_for() {
  std::cout << "Testing range-based for... ";
  Heap<int> h;

  h.push(5, 100);
  h.push(3, 200);
  h.push(8, 300);

  size_t count = 0;
  for (const auto& pair : h) {
    count++;
    assert(pair.second >= 100 && pair.second <= 300);
  }
  assert(count == 3);
  std::cout << "passed\n";
}

void test_priority_order() {
  std::cout << "Testing priority order extraction... ";
  Heap<int> h;

  h.push(10, 1000);
  h.push(50, 5000);
  h.push(30, 3000);
  h.push(20, 2000);
  h.push(40, 4000);

  assert(h.peek() == 5000);
  h.pop();  // priority 50
  assert(h.peek() == 4000);
  h.pop();  // priority 40
  assert(h.peek() == 3000);
  h.pop();  // priority 30
  assert(h.peek() == 2000);
  h.pop();  // priority 20
  assert(h.peek() == 1000);
  h.pop();  // priority 10

  assert(h.isEmpty());
  std::cout << "passed\n";
}

void test_rvalue_push() {
  std::cout << "Testing rvalue push... ";
  Heap<std::string> h;

  std::string s = "movable";
  h.push(10, std::move(s));

  assert(h.peek() == "movable");
  std::cout << "passed\n";
}

int main() {
  std::cout << "=== Heap Test Suite ===\n\n";

  try {
    test_constructor();
    test_max_heap_basic();
    test_min_heap();
    test_same_priorities();
    test_many_elements();
    test_copy_constructor();
    test_move_constructor();
    test_copy_assignment();
    test_move_assignment();
    test_clear();
    test_with_strings();
    test_exceptions();
    test_iterators();
    test_range_for();
    test_priority_order();
    test_rvalue_push();

    std::cout << "\n=== All tests passed! ===\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\n Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n Test failed with unknown exception\n";
    return 1;
  }
}
