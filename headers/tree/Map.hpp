#pragma once

#include <functional>
#include <utility>

#include "impl/RedBlackBST.tpp"

namespace aoi {

  template <typename Key, typename T, typename Compare = std::less<Key>>
  class Map {
   public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;

   private:
    using Tree = RedBlackBST<Key, T, Compare>;
    Tree tree_;

   public:
    using iterator = typename Tree::iterator;
    using const_iterator = typename Tree::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Map() = default;
    explicit Map(const Compare& comp) : tree_(comp) {}
    explicit Map(Compare&& comp) : tree_(std::move(comp)) {}

    template <typename InputIt>
    Map(InputIt first, InputIt last, const Compare& comp = Compare()) : tree_(comp) {
      insert(first, last);
    }

    Map(const Map&) = default;
    Map(Map&&) noexcept = default;

    Map(std::initializer_list<value_type> init, const Compare& comp = Compare()) : tree_(comp) { insert(init); }

    Map& operator=(const Map&) = default;
    Map& operator=(Map&&) noexcept = default;
    Map& operator=(std::initializer_list<value_type> ilist) {
      clear();
      insert(ilist);
      return *this;
    }
    ~Map() = default;


    iterator begin() noexcept { return tree_.begin(); }
    const_iterator begin() const noexcept { return tree_.begin(); }
    const_iterator cbegin() const noexcept { return tree_.cbegin(); }
    iterator end() noexcept { return tree_.end(); }
    const_iterator end() const noexcept { return tree_.end(); }
    const_iterator cend() const noexcept { return tree_.cend(); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }


    bool empty() const noexcept { return tree_.empty(); }
    size_type size() const noexcept { return tree_.size(); }
    size_type max_size() const noexcept { return static_cast<size_type>(-1); }

    mapped_type& at(const Key& key) { return tree_.at(key); }
    const mapped_type& at(const Key& key) const { return tree_.at(key); }
    mapped_type& operator[](const Key& key) { return tree_[key]; }
    mapped_type& operator[](Key&& key) { return tree_[std::move(key)]; }

    void clear() noexcept { tree_.clear(); }


    template <std::convertible_to<value_type> UV>
    std::pair<iterator, bool> insert(UV&& value) {
      return tree_.insert(std::forward<UV>(value));
    }
    template <std::convertible_to<value_type> UV>
    iterator insert(const_iterator hint, UV&& value) {
      (void)hint;
      return insert(std::forward<UV>(value)).first;
    }
    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
      for (; first != last; ++first) insert(*first);
    }
    void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }


    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
      value_type val(std::forward<Args>(args)...);
      return insert(std::move(val));
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
      (void)hint;
      return emplace(std::forward<Args>(args)...).first;
    }

    iterator erase(iterator pos) { return tree_.erase(pos); }
    iterator erase(const_iterator pos) { return tree_.erase(pos); }
    size_type erase(const Key& key) { return tree_.erase(key); }
    iterator erase(const_iterator first, const_iterator last) {
      while (first != last) first = erase(first);
      return iterator {&tree_, first.handle()};
    }

    void swap(Map& other) noexcept {
      using std::swap;
      swap(tree_, other.tree_);
    }


    iterator find(const Key& key) { return tree_.find(key); }
    const_iterator find(const Key& key) const { return tree_.find(key); }

    size_type count(const Key& key) const { return contains(key) ? 1 : 0; }
    bool contains(const Key& key) const { return tree_.contains(key); }


    bool operator==(const Map& other) const {
      return size() == other.size() && std::equal(begin(), end(), other.begin());
    }
    bool operator!=(const Map& other) const { return !(*this == other); }
    bool operator<(const Map& other) const {
      return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }
    bool operator<=(const Map& other) const { return !(other < *this); }
    bool operator>(const Map& other) const { return other < *this; }
    bool operator>=(const Map& other) const { return !(*this < other); }
  };

}
