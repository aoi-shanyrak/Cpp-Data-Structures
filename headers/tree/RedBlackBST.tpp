#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <deque>
#include <stack>
#include <stdexcept>
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


    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    void insert(UK key, UT value);

    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    void remove(UK&& key, UT&& value);


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


    void balance_after_insert(index_type q_idx);
    void balance_after_remove(index_type idx);

    void rotate_left(index_type x_idx);
    void rotate_right(index_type idx);


    struct BuildResult {
      index_type node;
      size_t height;
    };
    void shrink_to_fit();
    void extract_in_order(index_type idx, std::vector<std::pair<K, T>>& out);
    BuildResult build_balanced(const std::vector<std::pair<K, T>>& elements, size_t start, size_t end,
                               index_type parent);

    const Node& node_at(index_type idx) const { return *std::launder(reinterpret_cast<const Node*>(&data[idx])); }
    Node& node_at(index_type idx) { return const_cast<Node&>(std::as_const(*this).node_at(idx)); }

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
    index_type parent {nullindex};
    index_type* cur {&root};

    while (*cur != nullindex) {
      Node& node {node_at(*cur)};
      if (key < node.key) {
        cur = &node.left;
      } else if (key > node.key) {
        cur = &node.right;
      } else {
        node.value = std::forward<UT>(value);
        return;
      }
    }
    index_type new_idx {new_node(std::forward<UK>(key), std::forward<UT>(value))};
    node_at(new_idx).parent = parent;

    balance_after_insert(new_idx);
  }


  template <typename K, typename T>
  void RedBlackBST<K, T>::balance_after_insert(index_type q_idx) {
    while (q_idx != root && node_at(node_at(q_idx).parent).color == Color::Red) {
      index_type z_idx {node_at(q_idx).parent};
      index_type y_idx {node_at(z_idx).parent};

      bool z_is_left {z_idx == node_at(y_idx).left};
      index_type x_idx {z_is_left ? node_at(z_idx).right : node_at(z_idx).left};

      if (x_idx != nullindex && node_at(x_idx).color == Color::Red) {
        node_at(z_idx).color = Color::Black;
        node_at(x_idx).color = Color::Black;
        node_at(y_idx).color = Color::Red;
        q_idx = y_idx;
        continue;
      }

      if ((z_is_left && q_idx == node_at(z_idx).right) || (!z_is_left && q_idx == node_at(z_idx).left)) {
        if (z_is_left)
          rotate_left(z_idx);
        else
          rotate_right(z_idx);
        std::swap(q_idx, z_idx);
        y_idx = node_at(q_idx).parent;
      }

      node_at(z_idx).color = Color::Black;
      node_at(y_idx).color = Color::Red;
      if (z_is_left)
        rotate_right(y_idx);
      else
        rotate_left(y_idx);

      break;
    }
    node_at(root).color = Color::Black;
  }


  template <typename K, typename T>
  void RedBlackBST<K, T>::rotate_left(index_type x_idx) {
    Node& x {node_at(x_idx)};
    index_type y_idx {x.right};
    Node& y {node_at(y_idx)};

    x.right = y.left;
    if (y.left != nullindex) {
      node_at(y.left).parent = x_idx;
    }

    y.parent = x.parent;
    if (x.parent == nullindex) {
      root = y_idx;
    } else if (x_idx == node_at(x.parent).left) {
      node_at(x.parent).left = y_idx;
    } else {
      node_at(x.parent).right = y_idx;
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
      node_at(x.right).parent = y_idx;
    }

    x.parent = y.parent;
    if (y.parent == nullindex) {
      root = x_idx;
    } else if (y_idx == node_at(y.parent).right) {
      node_at(y.parent).right = x_idx;
    } else {
      node_at(y.parent).left = x_idx;
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
