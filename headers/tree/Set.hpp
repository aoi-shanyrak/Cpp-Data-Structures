#pragma once

#include <functional>
#include <utility>

#include "impl/RedBlackBST.tpp"

namespace aoi {

  namespace detail {
    struct Empty {};
  }

  template <typename Key, typename Compare = std::less<Key>>
  class Set {
   public:
    using key_type = Key;
    using value_type = Key;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using value_compare = Compare;
    using reference = const value_type&;
    using const_reference = const value_type&;

   private:
    using Tree = RedBlackBST<Key, detail::Empty, Compare>;
    Tree tree_;

   public:
    class iterator {
     public:
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = Key;
      using difference_type = std::ptrdiff_t;
      using reference = const Key&;
      using pointer = const Key*;

      iterator() = default;
      iterator(typename Tree::iterator it) : it_(it) {}

      reference operator*() const { return it_->first; }
      pointer operator->() const { return &it_->first; }

      iterator& operator++() {
        ++it_;
        return *this;
      }
      iterator operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
      }
      iterator& operator--() {
        --it_;
        return *this;
      }
      iterator operator--(int) {
        auto tmp = *this;
        --*this;
        return tmp;
      }

      bool operator==(const iterator& other) const { return it_ == other.it_; }
      bool operator!=(const iterator& other) const { return it_ != other.it_; }

      typename Tree::iterator base() const { return it_; }

     private:
      typename Tree::iterator it_;
    };

    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    Set() = default;
    explicit Set(const Compare& comp) : tree_(comp) {}
    explicit Set(Compare&& comp) : tree_(std::move(comp)) {}

    template <typename InputIt>
    Set(InputIt first, InputIt last, const Compare& comp = Compare()) : tree_(comp) {
      insert(first, last);
    }

    Set(const Set&) = default;
    Set(Set&&) noexcept = default;

    Set(std::initializer_list<value_type> init, const Compare& comp = Compare()) : tree_(comp) { insert(init); }

    Set& operator=(const Set&) = default;
    Set& operator=(Set&&) noexcept = default;
    Set& operator=(std::initializer_list<value_type> ilist) {
      clear();
      insert(ilist);
      return *this;
    }
    ~Set() = default;


    iterator begin() noexcept { return iterator(tree_.begin()); }
    const_iterator begin() const noexcept { return iterator(const_cast<Tree&>(tree_).begin()); }
    const_iterator cbegin() const noexcept { return iterator(const_cast<Tree&>(tree_).begin()); }
    iterator end() noexcept { return iterator(tree_.end()); }
    const_iterator end() const noexcept { return iterator(const_cast<Tree&>(tree_).end()); }
    const_iterator cend() const noexcept { return iterator(const_cast<Tree&>(tree_).end()); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }


    bool empty() const noexcept { return tree_.empty(); }
    size_type size() const noexcept { return tree_.size(); }
    size_type max_size() const noexcept { return static_cast<size_type>(-1); }

    void clear() noexcept { tree_.clear(); }


    template <std::convertible_to<value_type> UV>
    std::pair<iterator, bool> insert(UV&& value) {
      auto res = tree_.insert(std::forward<UV>(value), detail::Empty {});
      return {iterator(res.first), res.second};
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
      Key key(std::forward<Args>(args)...);
      return insert(std::move(key));
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
      (void)hint;
      return emplace(std::forward<Args>(args)...).first;
    }

    iterator erase(iterator pos) {
      auto next = tree_.erase(pos.base());
      return iterator(next);
    }
    size_type erase(const Key& key) { return tree_.erase(key); }
    iterator erase(const_iterator first, const_iterator last) {
      while (first != last) first = erase(first);
      return iterator(first.base());
    }

    void swap(Set& other) noexcept {
      using std::swap;
      swap(tree_, other.tree_);
    }

    iterator find(const Key& key) { return iterator(tree_.find(key)); }
    const_iterator find(const Key& key) const { return iterator(const_cast<Tree&>(tree_).find(key)); }
    size_type count(const Key& key) const { return contains(key) ? 1 : 0; }
    bool contains(const Key& key) const { return tree_.contains(key); }


    bool operator==(const Set& other) const {
      return size() == other.size() && std::equal(begin(), end(), other.begin());
    }
    bool operator!=(const Set& other) const { return !(*this == other); }
    bool operator<(const Set& other) const {
      return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }
    bool operator<=(const Set& other) const { return !(other < *this); }
    bool operator>(const Set& other) const { return other < *this; }
    bool operator>=(const Set& other) const { return !(*this < other); }
  };

}
