#include <cassert>
#include <iostream>
#include <string>

#include "../headers/array/Vector.tpp"

using namespace aoi;

void test_constructor() {
  std::cout << "Testing constructor... ";
  Vector<int> v(5, 42);
  assert(v.size() == 5);
  assert(v.capacity() >= 5);
  std::cout << "passed\n";
}

void test_copy_constructor() {
  std::cout << "Testing copy constructor... ";
  Vector<int> v1(3, 10);
  Vector<int> v2(v1);
  assert(v2.size() == 3);
  assert(v1.size() == v2.size());
  std::cout << "passed\n";
}

void test_move_constructor() {
  std::cout << "Testing move constructor... ";
  Vector<int> v1(5, 20);
  size_t old_size = v1.size();
  Vector<int> v2(std::move(v1));
  assert(v2.size() == old_size);
  assert(v1.size() == 0);
  std::cout << "passed\n";
}

void test_copy_assignment() {
  std::cout << "Testing copy assignment... ";
  Vector<int> v1(3, 100);
  Vector<int> v2(1, 50);
  v2 = v1;
  assert(v2.size() == 3);
  std::cout << "passed\n";
}

void test_move_assignment() {
  std::cout << "Testing move assignment... ";
  Vector<int> v1(5, 200);
  Vector<int> v2(1, 100);
  size_t old_size = v1.size();
  v2 = std::move(v1);
  assert(v2.size() == old_size);
  std::cout << "passed\n";
}

void test_push_back() {
  std::cout << "Testing push_back... ";
  Vector<int> v(0);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  assert(v.size() == 3);
  std::cout << "passed\n";
}

void test_pop_back() {
  std::cout << "Testing pop_back... ";
  Vector<int> v(3, 10);
  size_t old_size = v.size();
  v.pop_back();
  assert(v.size() == old_size - 1);
  std::cout << "passed\n";
}

void test_reserve() {
  std::cout << "Testing reserve... ";
  Vector<int> v(2, 5);
  v.reserve(10);
  assert(v.capacity() >= 10);
  assert(v.size() == 2);
  std::cout << "passed\n";
}

void test_resize() {
  std::cout << "Testing resize... ";
  Vector<int> v(5, 1);
  v.resize(10, 2);
  assert(v.size() == 10);
  v.resize(3);
  assert(v.size() == 3);
  std::cout << "passed\n";
}

void test_clear() {
  std::cout << "Testing clear... ";
  Vector<int> v(10, 42);
  v.clear();
  assert(v.size() == 0);
  std::cout << "passed\n";
}

void test_with_strings() {
  std::cout << "Testing with std::string... ";
  Vector<std::string> v(3, "hello");
  v.push_back("world");
  assert(v.size() == 4);
  std::cout << "passed\n";
}

void test_iterators() {
  std::cout << "Testing iterators... ";
  Vector<int> v(5, 10);
  int count = 0;
  for (auto it = v.begin(); it != v.end(); ++it) {
    count++;
  }
  assert(count == 5);
  std::cout << "passed\n";
}

void test_range_for() {
  std::cout << "Testing range-based for... ";
  Vector<int> v(3, 7);
  int sum = 0;
  for (const auto& val : v) {
    sum += val;
  }
  assert(sum == 21);
  std::cout << "passed\n";
}

int main() {
  std::cout << "=== Vector Test Suite ===\n\n";

  try {
    test_constructor();
    test_copy_constructor();
    test_move_constructor();
    test_copy_assignment();
    test_move_assignment();
    test_push_back();
    test_pop_back();
    test_reserve();
    test_resize();
    test_clear();
    test_with_strings();
    test_iterators();
    test_range_for();

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
