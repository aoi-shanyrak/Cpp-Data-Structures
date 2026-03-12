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
class Heap {
 public:
  using iterator = typename std::vector<std::pair<P, T>, Allocator>::iterator;
  using const_iterator = typename std::vector<std::pair<P, T>, Allocator>::const_iterator;

  Heap() = default;
  Heap(const Heap&) = default;
  Heap(Heap&&) noexcept = default;
  Heap& operator=(const Heap&) = default;
  Heap& operator=(Heap&&) noexcept = default;
  ~Heap() = default;

  const T& peek() const {
    if (isEmpty()) throw std::runtime_error("Heap::top(): heap is empty");
    return data[0].second;
  }

  std::pair<const T&, const P&> peekWithPriority() const {
    if (isEmpty()) throw std::runtime_error("Heap::top(): heap is empty");
    return data[0];
  }

  void push(P priority, const T& value) { push_impl(priority, value); }
  void push(P priority, T&& value) { push_impl(priority, std::move(value)); }

  std::pair<P, T> popWithPriority() {
    if (isEmpty()) throw std::runtime_error("Heap::popWithPriority(): heap is empty");
    auto top = std::move(data[0]);
    data[0] = std::move(data.back());
    data.pop_back();
    if (!isEmpty()) {
      heapDown(0);
    }
    return top;
  }

  void pop() {
    if (isEmpty()) {
      throw std::runtime_error("Heap::pop(): heap is empty");
    }
    data[0] = std::move(data.back());
    data.pop_back();
    if (!isEmpty()) {
      heapDown(0);
    }
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

  bool higherPriority(size_t a, size_t b) const noexcept(noexcept(comp(data[a].first, data[b].first))) {
    return comp(data[a].first, data[b].first);
  }

  void heapUp(size_t i) {
    while (i > 0) {
      size_t p = details::HeapParent(i);
      if (higherPriority(i, p)) {
        std::swap(data[i], data[p]);
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

      std::swap(data[i], data[best]);
      i = best;
    }
  }

  template <typename U>
  void push_impl(P priority, U&& value) {
    data.emplace_back(priority, std::forward<U>(value));
    try {
      heapUp(data.size() - 1);
    } catch (...) {
      data.pop_back();
      throw;
    }
  }
};
