#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <deque>
#include <stack>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace aoi {

  template <typename K, typename T>
  class RedBlackBST {
    using index_type = uint32_t;
    static constexpr index_type nullindex = static_cast<index_type>(-1);

    enum class Color { Red, Black };

    struct Node {
      index_type parent, left, right;
      K key;
      T value;
      Color color;
    };

   public:
    using key_type = K;
    using value_type = T;

    RedBlackBST() : data {}, freeList {}, root {nullindex} {}
    RedBlackBST(const RedBlackBST&) = default;
    RedBlackBST& operator=(const RedBlackBST&) = default;
    RedBlackBST(RedBlackBST&&) = default;
    RedBlackBST& operator=(RedBlackBST&&) = default;
    ~RedBlackBST() { clear(); }

    // TODO: min/max


    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    void insert(UK key, UT value);

    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    void remove(UK&& key, UT&& value) {
      SearchResult res {search(std::forward<UK>(key))};
      if (res.node == nullindex) return;
      remove_node(res.node, res.is_left);
    }


    bool empty() { return root == nullindex; }
    size_t size() { return data.size() - freeList.size(); }

    void clear() noexcept {
      clear_subtree(root);
      data.clear();
      freeList = {};
      root = nullindex;
    }


   private:
    using Storage = std::aligned_storage_t<sizeof(Node), alignof(Node)>;

    std::deque<Storage> data;
    std::stack<index_type> freeList;
    index_type root;


    struct SearchResult {
      index_type node;
      index_type parent;
      bool is_left;
    };
    template <std::convertible_to<K> UK>
    SearchResult search(UK&& key) const;

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


    const Node& node_at(index_type idx) const { return *std::launder(reinterpret_cast<const Node*>(&data[idx])); }
    Node& node_at(index_type idx) { return const_cast<Node&>(std::as_const(*this).node_at(idx)); }

    struct BuildResult {
      index_type node;
      size_t height;
    };
    void shrink_to_fit();
    void extract_in_order(index_type idx, std::vector<std::pair<K, T>>& out);
    BuildResult build_balanced(const std::vector<std::pair<K, T>>& elements, size_t start, size_t end,
                               index_type parent);

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


  template <typename K, typename T>
  template <std::convertible_to<K> UK, std::convertible_to<T> UT>
  void RedBlackBST<K, T>::insert(UK key, UT value) {
    SearchResult res {search(std::forward<UK>(key))};

    if (res.node != nullindex) {
      value_of(res.node) = std::forward<UT>(value);
      return;
    }

    index_type new_idx {new_node(std::forward<UK>(key), std::forward<UT>(value))};
    parent_of(new_idx) = res.parent;

    if (res.parent == nullindex) {
      root = new_idx;
      color_of(root) = Color::Black;
      return;

    } else if (res.is_left)
      left_of(res.parent) = new_idx;
    else
      right_of(res.parent) = new_idx;

    balance_after_insert(new_idx);
  }

  template <typename K, typename T>
  void RedBlackBST<K, T>::remove_node(index_type toDelete, bool is_left_toDelete) {
    index_type parent {parent_of(toDelete)};

    char amount_of_children {(left_of(toDelete) != nullindex) + (right_of(toDelete) != nullindex)};

    switch (amount_of_children) {
      case 2:
        SearchResult succ {get_extreme(right_of(toDelete), toDelete, &Node::left)};

        key_of(toDelete) = std::move(key_of(succ.node));
        value_of(toDelete) = std::move(value_of(succ.node));

        remove_node(succ.node, succ.is_left);
        break;

      case 1:
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

      case 0:
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

    if (size() < data.size() / 4) {
      shrink_to_fit();
    }
  }


  template <typename K, typename T>
  template <std::convertible_to<K> UK>
  auto RedBlackBST<K, T>::search(UK&& key) const -> SearchResult {
    index_type parent {nullindex};
    index_type cur {root};
    bool is_left {false};

    while (cur != nullindex) {
      const Node& node {node_at(cur)};
      if (key < node.key) {
        parent = cur;
        cur = node.left;
        is_left = true;

      } else if (key > node.key) {
        parent = cur;
        cur = node.right;
        is_left = false;

      } else {
        return {cur, parent, is_left};
      }
    }
    return {nullindex, parent, is_left};
  }


  template <typename K, typename T>
  void RedBlackBST<K, T>::balance_after_insert(index_type New) {
    while (New != root && color_of(parent_of(New)) == Color::Red) {
      index_type parent {parent_of(New)};
      index_type grandpa {parent_of(parent)};

      bool is_left_parent {parent == node_at(grandpa).left};
      index_type uncle {is_left_parent ? right_of(parent) : left_of(parent)};

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

  template <typename K, typename T>
  void RedBlackBST<K, T>::balance_after_delete(index_type parent, bool is_left_deleted) {
    while (parent != nullindex) {
      index_type sibling {is_left_deleted ? right_of(parent) : left_of(parent)};

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
        is_left_deleted = (grandpa != nullindex) && (parent_of(grandpa) == left_of(grandpa));
        parent = grandpa;
        continue;
      }

      if (is_left_deleted && sibling_right_black) {
        color_of(sibling_left) = Color::Black;
        color_of(sibling) = Color::Red;
        rotate_right(sibling);
        sibling = right_of(parent);

      } else if (!is_left_deleted && sibling_left_black) {
        color_of(sibling_right) = Color::Black;
        color_of(sibling) = Color::Red;
        rotate_left(sibling);
        sibling = left_of(parent);
      }

      sibling_left = left_of(sibling);
      sibling_right = right_of(sibling);

      color_of(sibling) = color_of(parent);
      color_of(parent) = Color::Black;
      if (is_left_deleted) {
        color_of(sibling_right) = Color::Black;
        rotate_left(parent);
      } else {
        color_of(sibling_left) = Color::Black;
        rotate_right(parent);
      }
      break;
    }
    color_of(root) = Color::Black;
  }


  template <typename K, typename T>
  void RedBlackBST<K, T>::rotate_left(index_type x_idx) {
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
    } else if (x_idx == parent_of(x.parent)) {
      left_of(x.parent) = y_idx;
    } else {
      right_of(x.parent) = y_idx;
    }

    y.left = x_idx;
    x.parent = y_idx;
  }

  template <typename K, typename T>
  void RedBlackBST<K, T>::rotate_right(index_type y_idx) {
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
    } else if (y_idx == parent_of(y.parent)) {
      parent_of(y.parent) = x_idx;
    } else {
      parent_of(y.parent) = x_idx;
    }

    x.right = y_idx;
    y.parent = x_idx;
  }


  template <typename K, typename T>
  void RedBlackBST<K, T>::shrink_to_fit() {
    std::vector<std::pair<K, T>> elements;
    elements.reserve(size());
    extract_in_order(root, elements);

    data.clear();
    freeList = {};

    root = build_balanced(elements, 0, elements.size() - 1, nullindex);
  }

  template <typename K, typename T>
  void RedBlackBST<K, T>::extract_in_order(index_type idx, std::vector<std::pair<K, T>>& out) {
    if (idx == nullindex) return;
    Node& node {node_at(idx)};

    extract_in_order(node.left, out);

    out.emplace_back(std::move(node.key), std::move(node.value));
    delete_node(idx);

    extract_in_order(node.right, out);
  }

  template <typename K, typename T>
  auto RedBlackBST<K, T>::build_balanced(const std::vector<std::pair<K, T>>& elements, size_t start, size_t end,
                                         index_type parent) -> BuildResult {
    if (start > end) return {nullindex, -1};

    size_t middle {(start + end) / 2};
    index_type idx {new_node(elements[middle].first, elements[middle].second)};
    Node& node {node_at(idx)};
    node.parent = parent;

    BuildResult left_res {build_balanced(elements, start, middle - 1, idx)};
    BuildResult right_res {build_balanced(elements, middle + 1, end, idx)};
    node.left = left_res.node;
    node.right = right_res.node;

    size_t height {std::max(left_res.height, right_res.height) + 1};

    node.color = (height % 2 == 1) ? Color::Red : Color::Black;

    return {idx, height};
  }


  template <typename K, typename T>
  template <std::convertible_to<K> UK, std::convertible_to<T> UT>
  auto RedBlackBST<K, T>::new_node(UK&& key, UT&& value) -> index_type {
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
