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
      size_t id;

      Node(P p, const T& v, size_t nodeId) : priority {std::move(p)}, value {v}, id {nodeId} {}
      Node(P p, T&& v, size_t nodeId) : priority {std::move(p)}, value {std::move(v)}, id {nodeId} {}
    };

  }

  template <typename T, typename P = int32_t, typename Compare = std::less<P>,
            typename Container = std::vector<details::Node<T, P>>>
  class BinaryHeap {

   public:
    using value_type = T;
    using priority_type = P;
    using compare_type = Compare;
    using Handle = size_t;

    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    explicit BinaryHeap(const Compare& comp = Compare(), const Container& container = Container())
        : comp {comp}, data {container} {
      indexMap.clear();
      for (size_t i = 0; i < data.size(); ++i) {
        const size_t id = nextGlobalId();
        data[i].id = id;
        ensureIndexMapSize(id);
        indexMap[id] = i;
      }
      heapify();
    }
    BinaryHeap(const BinaryHeap&) = delete;
    BinaryHeap& operator=(const BinaryHeap&) = delete;

    BinaryHeap(BinaryHeap&&) noexcept = default;
    BinaryHeap& operator=(BinaryHeap&&) noexcept = default;
    ~BinaryHeap() = default;

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
      const size_t removedId = data[0].id;
      indexMap[removedId] = kInvalidIndex;

      if (data.size() == 1) {
        data.pop_back();
        return;
      }

      data[0] = std::move(data.back());
      data.pop_back();
      indexMap[data[0].id] = 0;
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
        ensureIndexMapSize(data[idx].id);
        indexMap[data[idx].id] = idx;
      }

      other.data.clear();
      other.indexMap.clear();
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
      for (const auto& node : data) {
        if (node.id < indexMap.size()) {
          indexMap[node.id] = kInvalidIndex;
        }
      }
      data.clear();
    }


   private:
    Compare comp;
    Container data;
    std::vector<size_t> indexMap;

    static constexpr size_t kInvalidIndex = std::numeric_limits<size_t>::max();

    static size_t nextGlobalId() {
      static size_t nextId = 0;
      return nextId++;
    }

    void ensureIndexMapSize(size_t id) {
      if (id >= indexMap.size()) {
        indexMap.resize(id + 1, kInvalidIndex);
      }
    }


    size_t resolveHandle(Handle handle, const char* message) const {
      if (handle >= indexMap.size() || indexMap[handle] == kInvalidIndex || indexMap[handle] >= data.size()) {
        throw std::out_of_range(message);
      }
      return indexMap[handle];
    }


    void swapIndices(size_t i, size_t j) {
      std::swap(data[i], data[j]);
      indexMap[data[i].id] = i;
      indexMap[data[j].id] = j;
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
      const size_t id = nextGlobalId();
      ensureIndexMapSize(id);
      indexMap[id] = kInvalidIndex;

      data.emplace_back(priority, std::forward<U>(value), id);
      indexMap[id] = data.size() - 1;
      try {
        heapUp(data.size() - 1);
      } catch (...) {
        indexMap[id] = kInvalidIndex;
        data.pop_back();
        throw;
      }
      return id;
    }
  };

}
