#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../headers/heaps/FibonacciHeap.tpp"

using namespace aoi;

using FibMinHeap = FibonacciHeap<int, int, std::less<int>>;
using FibMaxHeap = FibonacciHeap<int, int, std::greater<int>>;

void test_basic_push_pop_min() {
  std::cout << "Testing basic push/pop order (min)... ";
  FibMinHeap h;

  h.push(5, 50);
  h.push(1, 10);
  h.push(3, 30);
  h.push(2, 20);

  const std::vector<int> expected {10, 20, 30, 50};
  for (int v : expected) {
    assert(!h.empty());
    assert(h.peek() == v);
    h.pop();
  }
  assert(h.empty());
  std::cout << "passed\n";
}

void test_basic_push_pop_max() {
  std::cout << "Testing basic push/pop order (max)... ";
  FibMaxHeap h;

  h.push(5, 50);
  h.push(1, 10);
  h.push(3, 30);
  h.push(2, 20);

  const std::vector<int> expected {50, 30, 20, 10};
  for (int v : expected) {
    assert(!h.empty());
    assert(h.peek() == v);
    h.pop();
  }
  assert(h.empty());
  std::cout << "passed\n";
}

void test_peek_with_priority() {
  std::cout << "Testing peekWithPriority... ";

  FibMinHeap minHeap;
  minHeap.push(10, 100);
  minHeap.push(2, 20);
  auto [minValue, minPriority] = minHeap.peekWithPriority();
  assert(minValue == 20);
  assert(minPriority == 2);

  FibMaxHeap maxHeap;
  maxHeap.push(10, 100);
  maxHeap.push(2, 20);
  auto [maxValue, maxPriority] = maxHeap.peekWithPriority();
  assert(maxValue == 100);
  assert(maxPriority == 10);

  std::cout << "passed\n";
}

void test_decrease_priority_min_and_validation() {
  std::cout << "Testing decreasePriority (min) and validation... ";
  FibMinHeap h;

  auto a = h.push(10, 1);
  auto b = h.push(20, 2);
  auto c = h.push(30, 3);
  (void)a;
  (void)b;

  h.decreasePriority(c, 5);
  assert(h.peek() == 3);

  bool threw = false;
  try {
    h.decreasePriority(c, 50);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  std::cout << "passed\n";
}

void test_decrease_priority_max() {
  std::cout << "Testing decreasePriority semantics (max)... ";
  FibMaxHeap h;

  auto low = h.push(1, 10);
  h.push(5, 50);

  h.decreasePriority(low, 10);
  assert(h.peek() == 10);

  std::cout << "passed\n";
}

void test_delete_key_min_and_max() {
  std::cout << "Testing delete_key for min and max... ";

  {
    FibMinHeap h;
    auto a = h.push(10, 100);
    auto b = h.push(5, 50);
    auto c = h.push(20, 200);
    (void)a;
    (void)c;

    h.delete_key(b);
    assert(h.peek() == 100);
    h.pop();
    assert(h.peek() == 200);
  }

  {
    FibMaxHeap h;
    auto a = h.push(10, 100);
    auto b = h.push(5, 50);
    auto c = h.push(20, 200);
    (void)a;
    (void)b;

    h.delete_key(c);
    assert(h.peek() == 100);
    h.pop();
    assert(h.peek() == 50);
  }

  std::cout << "passed\n";
}

void test_merge_and_handle_after_merge() {
  std::cout << "Testing merge, self-merge and handle stability... ";

  FibMinHeap a;
  FibMinHeap b;

  a.push(50, 500);
  auto hb = b.push(40, 400);
  b.push(60, 600);

  a.merge(b);
  assert(b.empty());

  a.decreasePriority(hb, 1);
  assert(a.peek() == 400);

  size_t count = 0;
  FibMinHeap probe;
  probe.push(3, 3);
  probe.push(1, 1);
  probe.push(2, 2);
  probe.merge(probe);
  while (!probe.empty()) {
    probe.pop();
    ++count;
  }
  assert(count == 3);

  std::cout << "passed\n";
}

void test_rvalue_and_exceptions() {
  std::cout << "Testing rvalue push and exceptions... ";

  FibonacciHeap<std::string, int, std::less<int>> h;
  std::string payload = "movable";
  h.push(10, std::move(payload));
  assert(h.peek() == "movable");

  h.pop();

  bool peekThrew = false;
  try {
    h.peek();
  } catch (const std::runtime_error&) {
    peekThrew = true;
  }
  assert(peekThrew);

  bool popThrew = false;
  try {
    h.pop();
  } catch (const std::runtime_error&) {
    popThrew = true;
  }
  assert(popThrew);

  bool nullDecreaseThrew = false;
  try {
    h.decreasePriority(nullptr, 0);
  } catch (const std::invalid_argument&) {
    nullDecreaseThrew = true;
  }
  assert(nullDecreaseThrew);

  bool nullDeleteThrew = false;
  try {
    h.delete_key(nullptr);
  } catch (const std::invalid_argument&) {
    nullDeleteThrew = true;
  }
  assert(nullDeleteThrew);

  std::cout << "passed\n";
}

template <typename HeapType, typename Compare>
void stress_against_sorted_model(const char* name, Compare comp, int rounds, int perRound) {
  std::cout << "Stress test (" << name << ")... ";

  std::mt19937 rng(123456);
  std::uniform_int_distribution<int> prioDist(-1000, 1000);
  std::uniform_int_distribution<int> valueDist(-100000, 100000);

  for (int r = 0; r < rounds; ++r) {
    HeapType h;
    std::vector<std::pair<int, int>> model;
    model.reserve(static_cast<size_t>(perRound));

    for (int i = 0; i < perRound; ++i) {
      int p = prioDist(rng);
      int v = valueDist(rng);
      h.push(p, v);
      model.push_back({p, v});
    }

    std::sort(model.begin(), model.end(), [&](const auto& lhs, const auto& rhs) {
      if (lhs.first != rhs.first) {
        return comp(lhs.first, rhs.first);
      }
      return lhs.second < rhs.second;
    });

    for (size_t i = 0; i < model.size(); ++i) {
      auto [value, priority] = h.peekWithPriority();
      (void)value;
      assert(priority == model[i].first);
      h.pop();
    }
    assert(h.empty());
  }

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== FibonacciHeap Test Suite ===\n\n";

  test_basic_push_pop_min();
  test_basic_push_pop_max();
  test_peek_with_priority();
  test_decrease_priority_min_and_validation();
  test_decrease_priority_max();
  test_delete_key_min_and_max();
  test_merge_and_handle_after_merge();
  test_rvalue_and_exceptions();

  stress_against_sorted_model<FibMinHeap>("min", std::less<int>(), 100, 150);
  stress_against_sorted_model<FibMaxHeap>("max", std::greater<int>(), 100, 150);

  std::cout << "\n=== FibonacciHeap tests passed! ===\n";
  return 0;
}
