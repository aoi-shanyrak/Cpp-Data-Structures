#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <new>
#include <string>
#include <utility>

#include "../headers/LinkedList.hpp"

using namespace aoi;

namespace {

  struct Counted {
    static int alive;
    int value;

    explicit Counted(int v) : value(v) { ++alive; }
    ~Counted() { --alive; }
  };

  int Counted::alive = 0;

  void test_allocate_construct_destroy() {
    std::cout << "Testing allocate/construct/destroy... ";

    NodeAllocator<Counted> alloc;

    Counted* ptr = alloc.allocate();
    new (ptr) Counted(42);

    assert(ptr->value == 42);
    assert(Counted::alive == 1);

    ptr->~Counted();
    alloc.deallocate(ptr);

    assert(Counted::alive == 0);
    std::cout << "passed\n";
  }

  void test_reuse_deallocated_node() {
    std::cout << "Testing reuse of deallocated nodes... ";

    NodeAllocator<int> alloc;

    int* first = alloc.allocate();
    alloc.deallocate(first);
    int* second = alloc.allocate();

    // LIFO freelist policy means the same address should be reused.
    assert(first == second);

    alloc.deallocate(second);
    std::cout << "passed\n";
  }

  void test_multiple_allocations() {
    std::cout << "Testing multiple allocations... ";

    NodeAllocator<int> alloc;
    int* ptrs[16] = {};

    for (int i = 0; i < 16; ++i) {
      ptrs[i] = alloc.allocate();
      *ptrs[i] = i;
    }

    for (int i = 0; i < 16; ++i) {
      assert(*ptrs[i] == i);
    }

    for (int i = 0; i < 16; ++i) {
      alloc.deallocate(ptrs[i]);
    }

    std::cout << "passed\n";
  }

  void test_alignment_for_large_type() {
    std::cout << "Testing alignment... ";

    struct alignas(64) BigAligned {
      char payload[64];
    };

    NodeAllocator<BigAligned> alloc;
    BigAligned* ptr = alloc.allocate();

    auto address = reinterpret_cast<std::uintptr_t>(ptr);
    assert(address % alignof(BigAligned) == 0);

    alloc.deallocate(ptr);
    std::cout << "passed\n";
  }

  void test_move_constructor() {
    std::cout << "Testing move constructor... ";

    NodeAllocator<int> src;
    int* ptr = src.allocate();
    *ptr = 123;

    NodeAllocator<int> dst(std::move(src));

    assert(*ptr == 123);

    dst.deallocate(ptr);
    std::cout << "passed\n";
  }

  void test_move_assignment() {
    std::cout << "Testing move assignment... ";

    NodeAllocator<std::string> src;
    std::string* ptr = src.allocate();
    new (ptr) std::string("node");

    NodeAllocator<std::string> dst;
    dst = std::move(src);

    assert(*ptr == "node");

    ptr->~basic_string();
    dst.deallocate(ptr);
    std::cout << "passed\n";
  }

}  // namespace

int main() {
  std::cout << "=== NodeAllocator Test Suite ===\n\n";

  try {
    test_allocate_construct_destroy();
    test_reuse_deallocated_node();
    test_multiple_allocations();
    test_alignment_for_large_type();
    test_move_constructor();
    test_move_assignment();

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
