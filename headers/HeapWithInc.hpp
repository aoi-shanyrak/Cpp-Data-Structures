#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace details {

inline size_t HeapLeftChild(size_t index) {
  return 2 * index + 1;
}

inline size_t HeapRightChild(size_t index) {
  return 2 * index + 2;
}

inline size_t HeapParent(size_t index) {
  return (index - 1) / 2;
}

}

template <typename T, typename P = int, typename Compare = std::greater<P>,
          typename Allocator = std::allocator<std::pair<P, T>>>
class HeapWithInc {
 public:
  using iterator = typename std::vector<std::pair<P, T>, Allocator>::iterator;
  using const_iterator = typename std::vector<std::pair<P, T>, Allocator>::const_iterator;

  explicit HeapWithInc(size_t maxSize) : indexMap(maxSize, static_cast<size_t>(-1)) {}
  HeapWithInc(const HeapWithInc&) = default;
  HeapWithInc(HeapWithInc&&) noexcept = default;
  HeapWithInc& operator=(const HeapWithInc&) = default;
  HeapWithInc& operator=(HeapWithInc&&) noexcept = default;
  ~HeapWithInc() = default;

  const T& peek() const {
    if (isEmpty()) throw std::runtime_error("Heap::top(): heap is empty");
    return data[0].second;
  }

  std::pair<const P&, const T&> peekWithPriority() const {
    if (isEmpty()) throw std::runtime_error("Heap::top(): heap is empty");
    return {data[0].first, data[0].second};
  }

  void push(P priority, const T& value) { push_impl(priority, value); }
  void push(P priority, T&& value) { push_impl(priority, std::move(value)); }

  std::pair<P, T> popWithPriority() {
    if (isEmpty()) throw std::runtime_error("Heap::popWithPriority(): heap is empty");
    auto top = data[0];
    pop();
    return top;
  }

  void pop() {
    if (isEmpty()) {
      throw std::runtime_error("Heap::pop(): heap is empty");
    }
    if (!indexMap.empty()) {
      indexMap[data[0].second] = static_cast<size_t>(-1);
    }
    data[0] = std::move(data.back());
    data.pop_back();
    if (!isEmpty()) {
      if (!indexMap.empty()) {
        indexMap[data[0].second] = 0;
      }
      heapDown(0);
    }
  }

  void increasePriorityByValue(T value, P newPriority) {
    if (indexMap.empty() || value >= indexMap.size()) {
      throw std::runtime_error("Heap::increasePriorityByValue(): indexMap not initialized");
    }
    size_t heapIndex = indexMap[value];
    if (heapIndex == static_cast<size_t>(-1)) {
      throw std::runtime_error("Heap::increasePriorityByValue(): value not in heap");
    }
    increasePriority(heapIndex, newPriority);
  }

  void increasePriority(size_t index, P newPriority) {
    if (index >= size()) {
      throw std::out_of_range("Heap::increasePriority(): index out of range");
    }
    data[index].first = newPriority;
    heapUp(index);
  }

  bool containsValue(T value) const {
    if (indexMap.empty() || value >= indexMap.size()) return false;
    return indexMap[value] != static_cast<size_t>(-1);
  }

  iterator begin() noexcept { return data.begin(); }
  iterator end() noexcept { return data.end(); }
  const_iterator begin() const noexcept { return data.begin(); }
  const_iterator end() const noexcept { return data.end(); }

  bool isEmpty() const noexcept { return data.empty(); }
  size_t size() const noexcept { return data.size(); }
  void clear() noexcept { data.clear(); }

 private:
  Compare comp;
  std::vector<std::pair<P, T>, Allocator> data;
  std::vector<size_t> indexMap;

  void swapIndices(size_t i, size_t j) {
    std::swap(data[i], data[j]);
    if (!indexMap.empty()) {
      indexMap[data[i].second] = i;
      indexMap[data[j].second] = j;
    }
  }

  bool higherPriority(size_t a, size_t b) const noexcept(noexcept(comp(data[a].first, data[b].first))) {
    return comp(data[a].first, data[b].first);
  }

  void heapUp(size_t i) {
    while (i > 0) {
      size_t p = details::HeapParent(i);
      if (higherPriority(i, p)) {
        swapIndices(i, p);
        i = p;
      } else
        break;
    }
  }

  void heapDown(size_t i) {
    while (true) {
      size_t left = details::HeapLeftChild(i);
      size_t right = details::HeapRightChild(i);
      size_t best = i;

      if (left < size() && higherPriority(left, best)) {
        best = left;
      }
      if (right < size() && higherPriority(right, best)) {
        best = right;
      }
      if (best == i) break;

      swapIndices(i, best);
      i = best;
    }
  }

  template <typename U>
  void push_impl(P priority, U&& value) {
    size_t newIndex = data.size();
    data.emplace_back(priority, std::forward<U>(value));
    if (!indexMap.empty()) {
      indexMap[data[newIndex].second] = newIndex;
    }
    try {
      heapUp(newIndex);
    } catch (...) {
      data.pop_back();
      if (!indexMap.empty()) {
        indexMap[data[newIndex].second] = static_cast<size_t>(-1);
      }
      throw;
    }
  }
};
