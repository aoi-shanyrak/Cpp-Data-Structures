#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <deque>
#include <iterator>
#include <stack>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace aoi {

  template <typename K, typename T, typename Compare = std::less<K>>
  class RedBlackBST {
    using index_type = uint32_t;

    enum class Color { Red, Black };

    struct Node {
      index_type parent, left, right;
      K key;
      [[no_unique_address]] T value;
      Color color;
    };


   public:
    using key_type = K;
    using mapped_type = T;
    using value_type = std::pair<const K, T>;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using Handle = index_type;
    static constexpr index_type nullindex = static_cast<index_type>(-1);


    class value_compare {
      friend class RedBlackBST;

     protected:
      Compare comp;
      value_compare(Compare c = Compare()) : comp(c) {}

     public:
      bool operator()(const value_type& lhs, const value_type& rhs) const { return comp(lhs.first, rhs.first); }
    };


    template <bool IsConst>
    class Iterator;
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    iterator begin() noexcept { return iterator {this, get_extreme(root, nullindex, &Node::left).node}; }
    iterator end() noexcept { return iterator {this, nullindex}; }
    const_iterator begin() const noexcept {
      return const_iterator {this, get_extreme(root, nullindex, &Node::left).node};
    }
    const_iterator end() const noexcept { return const_iterator {this, nullindex}; }
    const_iterator cbegin() const noexcept {
      return const_iterator {this, get_extreme(root, nullindex, &Node::left).node};
    }
    const_iterator cend() const noexcept { return const_iterator {this, nullindex}; }


    RedBlackBST() : data {}, freeList {}, root {nullindex}, valid_iterators_counter {0}, comp {} {}
    explicit RedBlackBST(const Compare& comp)
        : data {}, freeList {}, root {nullindex}, valid_iterators_counter {0}, comp {comp} {}
    explicit RedBlackBST(Compare&& comp)
        : data {}, freeList {}, root {nullindex}, valid_iterators_counter {0}, comp {std::move(comp)} {}

    RedBlackBST(const RedBlackBST& other) : RedBlackBST {other.comp} {
      if (other.root == nullindex) return;
      std::vector<std::pair<K, T>> elements;
      elements.reserve(other.size());
      other.traverse_in_order(other.root, elements);
      if (!elements.empty()) {
        root = build_balanced(elements, 0, elements.size() - 1, nullindex).node;
      }
    }
    RedBlackBST& operator=(const RedBlackBST& other) {
      if (this != &other) {
        RedBlackBST temp {other};
        *this = std::move(temp);
      }
      return *this;
    }
    RedBlackBST(RedBlackBST&& other) noexcept
        : data {std::move(other.data)}, freeList {std::move(other.freeList)}, root {other.root},
          valid_iterators_counter {other.valid_iterators_counter}, comp {std::move(other.comp)} {
      other.data = {};
      other.root = nullindex;
      other.freeList = {};
      other.valid_iterators_counter = 0;
      other.comp = Compare {};
    }

    RedBlackBST& operator=(RedBlackBST&& other) noexcept {
      if (this != &other) {
        clear();
        data = std::move(other.data);
        freeList = std::move(other.freeList);
        root = other.root;
        valid_iterators_counter = other.valid_iterators_counter;
        comp = std::move(other.comp);

        other.data = {};
        other.root = nullindex;
        other.freeList = {};
        other.valid_iterators_counter = 0;
        other.comp = Compare {};
      }
      return *this;
    }
    ~RedBlackBST() { clear(); }


    template <std::convertible_to<K> UK>
    iterator find(UK&& key) {
      index_type idx {search_impl(std::forward<UK>(key)).node};
      return iterator {this, idx};
    }

    template <std::convertible_to<K> UK>
    const_iterator find(UK&& key) const {
      index_type idx {search_impl(std::forward<UK>(key)).node};
      return const_iterator {this, idx};
    }

    template <std::convertible_to<K> UK>
    bool contains(UK&& key) const {
      return search_impl(std::forward<UK>(key)).node != nullindex;
    }

    template <std::convertible_to<K> UK>
    T& at(UK&& key) {
      auto it {find(std::forward<UK>(key))};
      if (it == end()) throw std::out_of_range("Key not found");
      return it->second;
    }

    template <std::convertible_to<K> UK>
    const T& at(UK&& key) const {
      auto it {find(std::forward<UK>(key))};
      if (it == end()) throw std::out_of_range("Key not found");
      return it->second;
    }

    template <std::convertible_to<K> UK>
    T& operator[](UK&& key) {
      auto res {search_handle(std::forward<UK>(key))};
      if (res == nullindex) {
        auto [it, inserted] {insert(std::forward<UK>(key), T {})};
        (void)inserted;
        return get_by_handle(it.handle());
      }
      return get_by_handle(res);
    }


    template <std::convertible_to<value_type> UV>
    std::pair<iterator, bool> insert(UV&& value) {
      return insert_impl(std::forward<UV>(value).first, std::forward<UV>(value).second);
    }

    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    std::pair<iterator, bool> insert(UK&& key, UT&& value) {
      return insert_impl(std::forward<UK>(key), std::forward<UT>(value));
    }


    template <std::convertible_to<K> UK>
    size_type erase(UK&& key) {
      SearchResult res {search_impl(std::forward<UK>(key))};
      if (res.node == nullindex) return 0;
      remove_node(res.node, res.is_left);
      return 1;
    }

    iterator erase(iterator pos) {
      if (pos == end()) return end();
      index_type toDelete {pos.handle()};
      index_type parent {parent_of(toDelete)};
      bool is_left_toDelete {(parent != nullindex) && (left_of(parent) == toDelete)};

      bool has_left {left_of(toDelete) != nullindex};
      bool has_right {right_of(toDelete) != nullindex};
      index_type next_handle {(has_left && has_right) ? toDelete : succ(toDelete)};

      remove_node(toDelete, is_left_toDelete);
      return (next_handle == nullindex) ? end() : iterator {this, next_handle};
    }
    iterator erase(const_iterator pos) { return erase(iterator {const_cast<RedBlackBST*>(this), pos.handle()}); }

    template <std::convertible_to<K> UK>
    void remove(UK&& key) {
      erase(std::forward<UK>(key));
    }


    const T& min() const {
      auto res {get_extreme(root, nullindex, &Node::left)};
      return value_of(res.node);
    }
    const T& max() const {
      auto res {get_extreme(root, nullindex, &Node::right)};
      return value_of(res.node);
    }

    template <std::convertible_to<K> UK>
    Handle search_handle(UK&& key) const {
      return search_impl(std::forward<UK>(key)).node;
    }

    Handle succ(Handle idx) const { return get_succ_or_pred(idx, true); }
    Handle pred(Handle idx) const { return get_succ_or_pred(idx, false); }
    T& get_by_handle(Handle idx) { return value_of(idx); }
    const T& get_by_handle(Handle idx) const { return value_of(idx); }


    bool empty() const noexcept { return root == nullindex; }
    size_type size() const noexcept { return data.size() - freeList.size(); }

    bool valid_iterators_exist() const noexcept { return valid_iterators_counter > 0; }

    void shrink_to_fit();

    void clear() noexcept {
      clear_subtree(root);
      data = {};
      freeList = {};
      root = nullindex;
      valid_iterators_counter = 0;
      comp = Compare {};
    }


   private:
    using Storage = std::aligned_storage_t<sizeof(Node), alignof(Node)>;

    std::deque<Storage> data;
    std::stack<index_type> freeList;
    index_type root;
    mutable size_type valid_iterators_counter;
    [[no_unique_address]] Compare comp;


    void acquire_iterator() const noexcept { ++valid_iterators_counter; }
    void release_iterator() const noexcept { --valid_iterators_counter; }

    struct SearchResult {
      index_type node;
      index_type parent;
      bool is_left;
    };
    template <std::convertible_to<K> UK>
    SearchResult search_impl(UK&& key) const;

    index_type get_succ_or_pred(index_type idx, bool get_succ) const;

    SearchResult get_extreme(index_type node, index_type parent, index_type Node::*direction) const {
      if (node == nullindex) return {nullindex, nullindex, false};

      index_type cur {node};
      while ((node_at(cur).*direction) != nullindex) {
        parent = cur;
        cur = node_at(cur).*direction;
      }
      bool is_left {(parent != nullindex) && (left_of(parent) == cur)};
      return {cur, parent, is_left};
    }


    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    std::pair<iterator, bool> insert_impl(UK&& key, UT&& value);

    void remove_node(index_type toDelete, bool is_left_toDelete);

    void balance_after_insert(index_type New);
    void balance_after_delete(index_type parent, bool is_left_deleted);

    void rotate_left(index_type x_idx);
    void rotate_right(index_type y_idx);


    K& key_of(index_type idx) { return node_at(idx).key; }
    T& value_of(index_type idx) { return node_at(idx).value; }
    index_type& parent_of(index_type idx) { return node_at(idx).parent; }
    index_type& left_of(index_type idx) { return node_at(idx).left; }
    index_type& right_of(index_type idx) { return node_at(idx).right; }
    Color& color_of(index_type idx) { return node_at(idx).color; }

    const K& key_of(index_type idx) const { return node_at(idx).key; }
    const T& value_of(index_type idx) const { return node_at(idx).value; }
    const index_type& parent_of(index_type idx) const { return node_at(idx).parent; }
    const index_type& left_of(index_type idx) const { return node_at(idx).left; }
    const index_type& right_of(index_type idx) const { return node_at(idx).right; }
    const Color& color_of(index_type idx) const { return node_at(idx).color; }


    const Node& node_at(index_type idx) const { return *std::launder(reinterpret_cast<const Node*>(&data[idx])); }
    Node& node_at(index_type idx) { return const_cast<Node&>(std::as_const(*this).node_at(idx)); }

    struct BuildResult {
      index_type node;
      size_t height;
    };
    void extract_in_order(index_type idx, std::vector<std::pair<K, T>>& out);
    BuildResult build_balanced(const std::vector<std::pair<K, T>>& elements, size_t start, size_t end,
                               index_type parent);
    void traverse_in_order(index_type idx, std::vector<std::pair<K, T>>& out) const {
      if (idx == nullindex) return;
      const Node& node {node_at(idx)};

      traverse_in_order(node.left, out);
      out.emplace_back(node.key, node.value);
      traverse_in_order(node.right, out);
    }

    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    index_type new_node(UK&& key, UT&& value);

    void delete_node(index_type idx) {
      node_at(idx).~Node();
      freeList.push(idx);
    }

    void clear_subtree(index_type idx) noexcept {
      if (idx == nullindex) return;
      Node& node {node_at(idx)};
      clear_subtree(node.left);
      clear_subtree(node.right);
      delete_node(idx);
    }
  };

  template <typename K, typename T, typename Compare>
  template <bool IsConst>
  class RedBlackBST<K, T, Compare>::Iterator {
    using tree_type = RedBlackBST<K, T, Compare>;
    using tree_pointer = std::conditional_t<IsConst, const tree_type*, tree_type*>;
    using node_type = typename tree_type::Node;
    using node_reference = std::conditional_t<IsConst, const node_type&, node_type&>;

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<const K, T>;

    struct reference {
      const K& first;
      std::conditional_t<IsConst, const T&, T&> second;

      friend bool operator==(const reference& lhs, const reference& rhs) {
        if (!(lhs.first == rhs.first)) return false;
        if constexpr (requires { lhs.second == rhs.second; }) {
          return lhs.second == rhs.second;
        }
        return true;
      }

      friend bool operator<(const reference& lhs, const reference& rhs) {
        if (lhs.first < rhs.first) return true;
        if (rhs.first < lhs.first) return false;
        if constexpr (requires { lhs.second < rhs.second; }) {
          return lhs.second < rhs.second;
        }
        return false;
      }
    };

    struct pointer {
      explicit pointer(reference ref) : ref_ {ref} {}
      const reference* operator->() const { return &ref_; }

     private:
      reference ref_;
    };


    Iterator() : tree {nullptr}, idx {tree_type::nullindex} {}

    Iterator(tree_pointer tree, typename tree_type::index_type idx_) noexcept : tree {tree}, idx {idx_} {
      if (tree) tree->acquire_iterator();
    }
    Iterator(const Iterator& other) noexcept : tree {other.tree}, idx {other.idx} {
      if (tree) tree->acquire_iterator();
    }
    template <bool OtherConst, typename = std::enable_if_t<IsConst && !OtherConst>>
    Iterator(const Iterator<OtherConst>& other) noexcept : tree {other.tree}, idx {other.idx} {
      if (tree) tree->acquire_iterator();
    }
    Iterator& operator=(const Iterator& other) noexcept {
      if (this != &other) {
        if (tree) tree->release_iterator();
        tree = other.tree;
        idx = other.idx;
        if (tree) tree->acquire_iterator();
      }
      return *this;
    }
    Iterator(Iterator&& other) noexcept
        : tree {std::exchange(other.tree, nullptr)}, idx {std::exchange(other.idx, tree_type::nullindex)} {}
    Iterator& operator=(Iterator&& other) noexcept {
      if (this != &other) {
        if (tree) tree->release_iterator();
        tree = std::exchange(other.tree, nullptr);
        idx = std::exchange(other.idx, tree_type::nullindex);
      }
      return *this;
    }
    ~Iterator() {
      if (tree) tree->release_iterator();
    }


    node_reference current_node() const { return tree->node_at(idx); }

    reference operator*() const {
      auto& node {current_node()};
      return reference {node.key, node.value};
    }
    pointer operator->() const { return pointer {operator*()}; }

    Iterator& operator++() {
      if (idx != tree_type::nullindex) idx = tree->succ(idx);
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp {*this};
      ++(*this);
      return tmp;
    }

    Iterator& operator--() {
      if (idx == tree_type::nullindex)
        idx = tree->get_extreme(tree->root, tree_type::nullindex, &tree_type::Node::right).node;
      else
        idx = tree->pred(idx);
      return *this;
    }

    Iterator operator--(int) {
      Iterator tmp {*this};
      --(*this);
      return tmp;
    }

    template <bool OtherConst>
    bool operator==(const Iterator<OtherConst>& other) const {
      return tree == other.tree && idx == other.idx;
    }

    template <bool OtherConst>
    bool operator!=(const Iterator<OtherConst>& other) const {
      return !(*this == other);
    }

    Handle handle() const { return idx; }


   private:
    tree_pointer tree;
    typename tree_type::index_type idx;

    friend class RedBlackBST<K, T>;
    template <bool>
    friend class Iterator;
  };


  template <typename K, typename T, typename Compare>
  auto RedBlackBST<K, T, Compare>::get_succ_or_pred(index_type idx, bool get_succ) const -> index_type {
    if (idx == nullindex) return nullindex;

    index_type Node::*direction {get_succ ? &Node::right : &Node::left};

    if (node_at(idx).*direction != nullindex) {
      index_type Node::*extreme_direction {get_succ ? &Node::left : &Node::right};
      auto res {get_extreme(node_at(idx).*direction, idx, extreme_direction)};
      return res.node;
    }

    index_type parent {parent_of(idx)};
    index_type cur {idx};

    while (parent != nullindex && cur == node_at(parent).*direction) {
      cur = parent;
      parent = parent_of(parent);
    }
    return parent;
  }


  template <typename K, typename T, typename Compare>
  template <std::convertible_to<K> UK, std::convertible_to<T> UT>
  auto RedBlackBST<K, T, Compare>::insert_impl(UK&& key, UT&& value) -> std::pair<iterator, bool> {
    SearchResult res {search_impl(std::forward<UK>(key))};

    if (res.node != nullindex) {
      value_of(res.node) = std::forward<UT>(value);
      return {iterator(this, res.node), false};
    }

    index_type new_idx {new_node(std::forward<UK>(key), std::forward<UT>(value))};
    parent_of(new_idx) = res.parent;

    if (res.parent == nullindex) {
      root = new_idx;
      color_of(root) = Color::Black;

    } else if (res.is_left)
      left_of(res.parent) = new_idx;
    else
      right_of(res.parent) = new_idx;

    balance_after_insert(new_idx);
    return {iterator(this, new_idx), true};
  }

  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::remove_node(index_type toDelete, bool is_left_toDelete) {
    index_type parent {parent_of(toDelete)};

    int amount_of_children {(left_of(toDelete) != nullindex) + (right_of(toDelete) != nullindex)};

    switch (amount_of_children) {
      case 2: {
        SearchResult succ {get_extreme(right_of(toDelete), toDelete, &Node::left)};

        key_of(toDelete) = std::move(key_of(succ.node));
        value_of(toDelete) = std::move(value_of(succ.node));

        remove_node(succ.node, succ.is_left);
        break;
      }

      case 1: {
        bool child_is_left {left_of(toDelete) != nullindex};
        index_type child {child_is_left ? left_of(toDelete) : right_of(toDelete)};

        if (parent == nullindex) {
          root = child;
          parent_of(child) = nullindex;
          color_of(child) = Color::Black;
          delete_node(toDelete);
          return;
        }

        parent_of(child) = parent;
        color_of(child) = Color::Black;
        if (is_left_toDelete) {
          left_of(parent) = child;
        } else {
          right_of(parent) = child;
        }
        delete_node(toDelete);

        break;
      }

      case 0: {
        if (parent == nullindex) {
          delete_node(toDelete);
          root = nullindex;
          return;
        }

        if (is_left_toDelete) {
          left_of(parent) = nullindex;
        } else {
          right_of(parent) = nullindex;
        }
        Color colorToDelete {color_of(toDelete)};
        delete_node(toDelete);

        if (colorToDelete == Color::Black) {
          balance_after_delete(parent, is_left_toDelete);
        }
      }
    }
    if (size() < data.size() / 4) {
      shrink_to_fit();
    }
  }


  template <typename K, typename T, typename Compare>
  template <std::convertible_to<K> UK>
  auto RedBlackBST<K, T, Compare>::search_impl(UK&& key) const -> SearchResult {
    index_type parent {nullindex};
    index_type cur {root};
    bool is_left {false};

    while (cur != nullindex) {
      const Node& node {node_at(cur)};
      if (comp(key, node.key)) {
        parent = cur;
        cur = node.left;
        is_left = true;

      } else if (comp(node.key, key)) {
        parent = cur;
        cur = node.right;
        is_left = false;

      } else {
        return {cur, parent, is_left};
      }
    }
    return {nullindex, parent, is_left};
  }


  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::balance_after_insert(index_type New) {
    while (New != root && color_of(parent_of(New)) == Color::Red) {
      index_type parent {parent_of(New)};
      index_type grandpa {parent_of(parent)};

      bool is_left_parent {parent == node_at(grandpa).left};
      index_type uncle {is_left_parent ? right_of(grandpa) : left_of(grandpa)};

      if (uncle != nullindex && color_of(uncle) == Color::Red) {
        color_of(parent) = Color::Black;
        color_of(uncle) = Color::Black;
        color_of(grandpa) = Color::Red;
        New = grandpa;
        continue;
      }

      if ((is_left_parent && New == right_of(parent)) || (!is_left_parent && New == left_of(parent))) {
        if (is_left_parent)
          rotate_left(parent);
        else
          rotate_right(parent);
        std::swap(New, parent);
        grandpa = parent_of(New);
      }

      color_of(parent) = Color::Black;
      color_of(grandpa) = Color::Red;
      if (is_left_parent)
        rotate_right(grandpa);
      else
        rotate_left(grandpa);

      break;
    }
    color_of(root) = Color::Black;
  }

  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::balance_after_delete(index_type parent, bool is_left_deleted) {
    while (parent != nullindex) {
      index_type sibling {is_left_deleted ? right_of(parent) : left_of(parent)};

      if (sibling == nullindex) {
        index_type grandpa {parent_of(parent)};
        is_left_deleted = (grandpa != nullindex) && (parent == left_of(grandpa));
        parent = grandpa;
        continue;
      }

      if (color_of(sibling) == Color::Red) {
        color_of(sibling) = Color::Black;
        color_of(parent) = Color::Red;

        if (is_left_deleted)
          rotate_left(parent);
        else
          rotate_right(parent);

        continue;
      }

      index_type sibling_left {left_of(sibling)};
      index_type sibling_right {right_of(sibling)};
      bool sibling_left_black {(sibling_left == nullindex) || (color_of(sibling_left) == Color::Black)};
      bool sibling_right_black {(sibling_right == nullindex) || (color_of(sibling_right) == Color::Black)};

      if (sibling_left_black && sibling_right_black) {
        color_of(sibling) = Color::Red;
        index_type grandpa {parent_of(parent)};
        is_left_deleted = (grandpa != nullindex) && (parent == left_of(grandpa));
        parent = grandpa;
        continue;
      }

      if (is_left_deleted && sibling_right_black) {
        if (sibling_left != nullindex) color_of(sibling_left) = Color::Black;
        color_of(sibling) = Color::Red;
        rotate_right(sibling);
        sibling = right_of(parent);

      } else if (!is_left_deleted && sibling_left_black) {
        if (sibling_right != nullindex) color_of(sibling_right) = Color::Black;
        color_of(sibling) = Color::Red;
        rotate_left(sibling);
        sibling = left_of(parent);
      }

      sibling_left = left_of(sibling);
      sibling_right = right_of(sibling);

      color_of(sibling) = color_of(parent);
      color_of(parent) = Color::Black;
      if (is_left_deleted) {
        if (sibling_right != nullindex) color_of(sibling_right) = Color::Black;
        rotate_left(parent);
      } else {
        if (sibling_left != nullindex) color_of(sibling_left) = Color::Black;
        rotate_right(parent);
      }
      break;
    }
    if (root != nullindex) color_of(root) = Color::Black;
  }


  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::rotate_left(index_type x_idx) {
    Node& x {node_at(x_idx)};
    index_type y_idx {x.right};
    Node& y {node_at(y_idx)};

    x.right = y.left;
    if (y.left != nullindex) {
      parent_of(y.left) = x_idx;
    }

    y.parent = x.parent;
    if (x.parent == nullindex) {
      root = y_idx;
    } else if (x_idx == left_of(x.parent)) {
      left_of(x.parent) = y_idx;
    } else {
      right_of(x.parent) = y_idx;
    }

    y.left = x_idx;
    x.parent = y_idx;
  }

  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::rotate_right(index_type y_idx) {
    Node& y {node_at(y_idx)};
    index_type x_idx {y.left};
    Node& x {node_at(x_idx)};

    y.left = x.right;
    if (x.right != nullindex) {
      parent_of(x.right) = y_idx;
    }

    x.parent = y.parent;
    if (y.parent == nullindex) {
      root = x_idx;
    } else if (y_idx == left_of(y.parent)) {
      left_of(y.parent) = x_idx;
    } else {
      right_of(y.parent) = x_idx;
    }

    x.right = y_idx;
    y.parent = x_idx;
  }


  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::shrink_to_fit() {
    std::vector<std::pair<K, T>> elements;
    elements.reserve(size());
    extract_in_order(root, elements);

    data.clear();
    freeList = {};
    if (elements.empty()) {
      root = nullindex;
      return;
    }

    root = build_balanced(elements, 0, elements.size() - 1, nullindex).node;
    color_of(root) = Color::Black;
  }

  template <typename K, typename T, typename Compare>
  void RedBlackBST<K, T, Compare>::extract_in_order(index_type idx, std::vector<std::pair<K, T>>& out) {
    if (idx == nullindex) return;
    Node& node {node_at(idx)};
    auto right {right_of(idx)};

    extract_in_order(node.left, out);

    out.emplace_back(std::move(node.key), std::move(node.value));
    delete_node(idx);

    extract_in_order(right, out);
  }

  template <typename K, typename T, typename Compare>
  auto RedBlackBST<K, T, Compare>::build_balanced(const std::vector<std::pair<K, T>>& elements, size_t start,
                                                  size_t end, index_type parent) -> BuildResult {
    if (start > end) return {nullindex, 0};

    size_t middle {(start + end) / 2};
    index_type idx {new_node(elements[middle].first, elements[middle].second)};
    Node& node {node_at(idx)};
    node.parent = parent;

    BuildResult left_res {nullindex, 0};
    BuildResult right_res {nullindex, 0};

    if (middle > start) {
      left_res = build_balanced(elements, start, middle - 1, idx);
    }
    if (middle < end) {
      right_res = build_balanced(elements, middle + 1, end, idx);
    }

    node.left = left_res.node;
    node.right = right_res.node;

    size_t height {std::max(left_res.height, right_res.height) + 1};

    node.color = (height % 2 == 1) ? Color::Red : Color::Black;

    return {idx, height};
  }


  template <typename K, typename T, typename Compare>
  template <std::convertible_to<K> UK, std::convertible_to<T> UT>
  auto RedBlackBST<K, T, Compare>::new_node(UK&& key, UT&& value) -> index_type {
    index_type idx;
    if (!freeList.empty()) {
      idx = freeList.top();
      freeList.pop();
    } else {
      idx = static_cast<index_type>(data.size());
      data.emplace_back();
    }
    new (&data[idx]) Node {nullindex, nullindex, nullindex, std::forward<UK>(key), std::forward<UT>(value), Color::Red};

    return idx;
  }
}
