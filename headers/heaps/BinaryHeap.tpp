#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace aoi {

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

  template <typename T, typename P = int32_t, typename Compare = std::less<P>,
            typename Container = std::vector<std::pair<P, T>>>
  class Heap {

   public:
    using value_type = T;
    using priority_type = P;
    using compare_type = Compare;
    using allocator_type = A;

    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    Heap() = default;
    Heap(const Heap&) = default;
    Heap(Heap&&) noexcept = default;
    Heap& operator=(const Heap&) = default;
    Heap& operator=(Heap&&) noexcept = default;
    virtual ~Heap() = default;

    const T& peek() const {
      if (isEmpty()) throw std::runtime_error("Heap::peek(): heap is empty");
      return data[0].second;
    }
    std::pair<const T&, const P&> peekWithPriority() const {
      if (isEmpty()) throw std::runtime_error("Heap::peekWithPriority(): heap is empty");
      return data[0];
    }

    void push(P priority, const T& value) { push_impl(priority, value); }
    void push(P priority, T&& value) { push_impl(priority, std::move(value)); }

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

    void merge(Heap& other) {
      if (this == &other) {
        return;
      }
      Container combined;
      combined.reserve(data.size() + other.data.size());

      for (auto& x : data) {
        combined.push_back(x);
      }
      for (auto& x : other.data) {
        combined.push_back(x);
      }
      data.clear();
      other.data.clear();

      data = std::move(combined);
      heapify();
    }

    iterator begin() noexcept { return data.begin(); }
    iterator end() noexcept { return data.end(); }
    const_iterator begin() const noexcept { return data.begin(); }
    const_iterator end() const noexcept { return data.end(); }

    bool isEmpty() const noexcept { return data.empty(); }
    size_t size() const noexcept { return data.size(); }

    virtual void clear() noexcept { data.clear(); }


   protected:
    Compare comp;
    Container data;

    virtual void swapIndices(size_t i, size_t j) { std::swap(data[i], data[j]); }

    bool higherPriority(size_t a, size_t b) const noexcept(noexcept(comp(data[a].first, data[b].first))) {
      return comp(data[a].first, data[b].first);
    }

    void heapify() {
      size_t n = size();
      for (size_t i = n; i-- > 0;) {
        heapDown(i);
      }
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
      try {
        heapUp(newIndex);
      } catch (...) {
        data.pop_back();
        throw;
      }
    }
  };


  template <typename T, typename P = int32_t, typename Compare = std::less<P>,
            typename Container = std::vector<std::pair<P, T>>>
  class HeapDecreasing : public Heap<T, P, Compare, Container> {
    using Base = Heap<T, P, Compare, Container>;

   public:
    using Handle = std::pair<P, T>*;

    HeapDecreasing() = default;
    HeapDecreasing(const HeapDecreasing&) = default;
    HeapDecreasing(HeapDecreasing&&) noexcept = default;
    HeapDecreasing& operator=(const HeapDecreasing&) = default;
    HeapDecreasing& operator=(HeapDecreasing&&) noexcept = default;
    ~HeapDecreasing() = default;

    void push(P priority, const T& value) = delete;
    void push(P priority, T&& value) = delete;

    void clear() noexcept override {
      Base::clear();
      ptrMap.clear();
    }


   private:
    std::vector<Handle> ptrMap;

    void push_impl(P priority, U&& value) = delete;
  };
}
