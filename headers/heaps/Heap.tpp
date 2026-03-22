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

    std::pair<P, T> popWithPriority() {
      if (isEmpty()) throw std::runtime_error("Heap::popWithPriority(): heap is empty");
      onElementRemoved(0);
      auto top = std::move(data[0]);
      data[0] = std::move(data.back());
      data.pop_back();
      if (!isEmpty()) {
        onElementMovedOnTop();
        heapDown(0);
      }
      return top;
    }

    void pop() {
      if (isEmpty()) {
        throw std::runtime_error("Heap::pop(): heap is empty");
      }
      onElementRemoved(0);
      data[0] = std::move(data.back());
      data.pop_back();
      if (!isEmpty()) {
        onElementMovedOnTop();
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
      rebuildIndexMap();
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

    virtual void rebuildIndexMap() {}
    virtual void swapIndices(size_t i, size_t j) { std::swap(data[i], data[j]); }
    virtual void onElementMovedOnTop() {}
    virtual void onElementAdded(size_t) {}
    virtual void onElementRemoved(size_t) {}

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
        onElementAdded(newIndex);
        heapUp(newIndex);
      } catch (...) {
        onElementRemoved(newIndex);
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
    HeapDecreasing() = default;
    explicit HeapDecreasing(size_t maxSize) : indexMap(maxSize, static_cast<size_t>(-1)) {}
    HeapDecreasing(const HeapDecreasing&) = default;
    HeapDecreasing(HeapDecreasing&&) noexcept = default;
    HeapDecreasing& operator=(const HeapDecreasing&) = default;
    HeapDecreasing& operator=(HeapDecreasing&&) noexcept = default;
    ~HeapDecreasing() = default;

    void decreasePriorityByValue(const T& value, P newPriority) {
      if (!isValueIndexValid(value)) {
        throw std::runtime_error("Heap::decreasePriorityByValue(): indexMap not initialized");
      }
      size_t heapIndex = indexMap[static_cast<size_t>(value)];
      if (heapIndex == static_cast<size_t>(-1)) {
        throw std::runtime_error("Heap::decreasePriorityByValue(): value not in heap");
      }
      decreasePriority(heapIndex, newPriority);
    }

    void decreasePriority(size_t index, P newPriority) {
      if (index >= Base::size()) {
        throw std::out_of_range("Heap::decreasePriority(): index out of range");
      }
      Base::data[index].first = newPriority;
      Base::heapUp(index);
    }

    void deleteByValue(const T& value) {
      if (Base::isEmpty()) {
        throw std::runtime_error("Heap::deleteByValue(): heap is empty");
      }
      if (!isValueIndexValid(value)) {
        throw std::runtime_error("Heap::deleteByValue(): indexMap not initialized");
      }
      P extremePriority = Base::comp(std::numeric_limits<P>::max(), std::numeric_limits<P>::lowest())
                              ? std::numeric_limits<P>::max()
                              : std::numeric_limits<P>::lowest();
      decreasePriorityByValue(value, extremePriority);
      Base::pop();
    }

    bool containsValue(const T& value) const {
      if (!isValueIndexValid(value)) return false;
      return indexMap[static_cast<size_t>(value)] != static_cast<size_t>(-1);
    }

    void clear() noexcept override {
      Base::clear();
      indexMap.clear();
    }


   private:
    std::vector<size_t> indexMap;

    void rebuildIndexMap() override {
      indexMap.clear();
      for (size_t i = 0; i < Base::size(); ++i) {
        onElementAdded(i);
      }
    }

    void swapIndices(size_t i, size_t j) {
      std::swap(Base::data[i], Base::data[j]);
      if (!indexMap.empty()) {
        indexMap[Base::data[i].second] = i;
        indexMap[Base::data[j].second] = j;
      }
    }

    void onElementMovedOnTop() override {
      if (!indexMap.empty()) {
        indexMap[Base::data[0].second] = 0;
      }
    }
    void onElementAdded(size_t newIndex) override {
      ensureIndexMapSize(Base::data[newIndex].second);
      if (!indexMap.empty()) {
        indexMap[Base::data[newIndex].second] = newIndex;
      }
    }
    void onElementRemoved(size_t index) override {
      if (!indexMap.empty()) {
        indexMap[Base::data[index].second] = static_cast<size_t>(-1);
      }
    }

    bool isValueIndexValid(const T& value) const {
      if (indexMap.empty()) {
        return false;
      }
      if constexpr (std::is_signed_v<T>) {
        if (value < 0) {
          return false;
        }
      }
      return static_cast<size_t>(value) < indexMap.size();
    }

    void ensureIndexMapSize(const T& value) {
      size_t idx = static_cast<size_t>(value);
      if (indexMap.size() <= idx) {
        indexMap.resize(idx + 1, static_cast<size_t>(-1));
      }
    }
  };
}
