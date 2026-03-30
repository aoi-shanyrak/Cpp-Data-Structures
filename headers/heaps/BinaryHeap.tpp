#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
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

    template <typename T, typename P>
    struct Node {
      P priority;
      T value;
      size_t* handle;

      Node(P p, const T& v, size_t* h) : priority {std::move(p)}, value {v}, handle {h} {}
      Node(P p, T&& v, size_t* h) : priority {std::move(p)}, value {std::move(v)}, handle {h} {}
    };

  }

  template <typename T, typename P = int32_t, typename Compare = std::less<P>,
            typename Container = std::vector<details::Node<T, P>>>
  class BinaryHeap {

   public:
    using value_type = T;
    using priority_type = P;
    using compare_type = Compare;
    using Handle = size_t*;

    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    explicit BinaryHeap(const Compare& comp = Compare(), const Container& container = Container())
        : comp {comp}, data {container} {
      for (size_t i = 0; i < data.size(); ++i) {
        data[i].handle = new size_t(i);
      }
      heapify();
    }
    BinaryHeap(const BinaryHeap&) = delete;
    BinaryHeap& operator=(const BinaryHeap&) = delete;

    BinaryHeap(BinaryHeap&&) noexcept = default;
    BinaryHeap& operator=(BinaryHeap&&) noexcept = default;
    ~BinaryHeap() {
      clear();
      for (Handle h : retiredHandles) {
        delete h;
      }
      retiredHandles.clear();
    }

    const T& peek() const {
      if (empty()) throw std::runtime_error("BinaryHeap::peek(): heap is empty");
      return data[0].value;
    }
    std::pair<const T&, const P&> peekWithPriority() const {
      if (empty()) throw std::runtime_error("BinaryHeap::peekWithPriority(): heap is empty");
      return {data[0].value, data[0].priority};
    }

    Handle push(P priority, const T& value) { return push_impl(priority, value); }
    Handle push(P priority, T&& value) { return push_impl(priority, std::move(value)); }

    void pop() {
      if (empty()) {
        throw std::runtime_error("BinaryHeap::pop(): heap is empty");
      }
      const size_t last = data.size() - 1;
      if (last != 0) {
        swapIndices(0, last);
      }
      retireNodeHandle(last);
      data.pop_back();
      if (!empty()) {
        heapDown(0);
      }
    }

    void merge(BinaryHeap& other) {
      if (this == &other) {
        return;
      }
      data.reserve(data.size() + other.data.size());
      for (auto& node : other.data) {
        data.push_back(std::move(node));
        const size_t idx = data.size() - 1;
        if (data[idx].handle) {
          *data[idx].handle = idx;
        }
      }
      other.data.clear();
      heapify();
    }

    void decreasePriority(Handle node, priority_type newPriority) {
      const size_t index = resolveHandle(node, "BinaryHeap::decreasePriority(): invalid node handle");
      data[index].priority = newPriority;
      heapUp(index);
    }

    void delete_key(Handle node) {
      const size_t index = resolveHandle(node, "BinaryHeap::delete_key(): invalid node handle");
      heapUp(index, true);
      pop();
    }

    iterator begin() noexcept { return data.begin(); }
    iterator end() noexcept { return data.end(); }
    const_iterator begin() const noexcept { return data.begin(); }
    const_iterator end() const noexcept { return data.end(); }

    bool empty() const noexcept { return data.empty(); }
    size_t size() const noexcept { return data.size(); }

    void clear() noexcept {
      for (size_t i = 0; i < data.size(); ++i) {
        retireNodeHandle(i);
      }
      data.clear();
    }


   private:
    Compare comp;
    Container data;
    std::vector<Handle> retiredHandles;

    static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();


    void retireNodeHandle(size_t index) noexcept {
      Handle h = data[index].handle;
      if (!h) {
        return;
      }
      *h = InvalidIndex;
      retiredHandles.push_back(h);
      data[index].handle = nullptr;
    }

    size_t resolveHandle(Handle handle, const char* message) const {
      if (handle == nullptr) {
        throw std::out_of_range(message);
      }
      const size_t index = *handle;
      if (index == InvalidIndex || index >= data.size()) {
        throw std::out_of_range(message);
      }
      if (data[index].handle == nullptr || data[index].handle != handle) {
        throw std::out_of_range(message);
      }
      return index;
    }

    void swapIndices(size_t i, size_t j) {
      std::swap(data[i], data[j]);
      *data[i].handle = i;
      *data[j].handle = j;
    }

    bool higherPriority(size_t a, size_t b) const noexcept(noexcept(comp(data[a].priority, data[b].priority))) {
      return comp(data[a].priority, data[b].priority);
    }

    void heapify() {
      size_t n = size();
      for (size_t i = n; i-- > 0;) {
        heapDown(i);
      }
    }

    void heapUp(size_t i, bool alwaysSwap = false) {
      while (i > 0) {
        size_t p = details::HeapParent(i);
        if (higherPriority(i, p) or alwaysSwap) {
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
    Handle push_impl(P priority, U&& value) {
      const size_t index = data.size();
      Handle h = new size_t(index);

      try {
        data.emplace_back(priority, std::forward<U>(value), h);
      } catch (...) {
        delete h;
        throw;
      }
      try {
        heapUp(index);
      } catch (...) {
        retireNodeHandle(data.size() - 1);
        data.pop_back();
        throw;
      }
      return h;
    }
  };

}
