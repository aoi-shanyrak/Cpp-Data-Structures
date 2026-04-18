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
      bool is_left {(parent != nullindex) && (node_at(parent).left == cur)};
      return {cur, parent, is_left};
    }


    void remove_node(index_type toDelete_idx, bool is_left_toDelete_for_parent) {
      index_type parent_idx {node_at(toDelete_idx).parent};

      char amount_of_children {(node_at(toDelete_idx).left != nullindex) + (node_at(toDelete_idx).right != nullindex)};

      switch (amount_of_children) {
        case 2:
          SearchResult succ {get_extreme(node_at(toDelete_idx).right, toDelete_idx, &Node::left)};

          node_at(toDelete_idx).key = std::move(node_at(succ.node).key);
          node_at(toDelete_idx).value = std::move(node_at(succ.node).value);

          remove_node(succ.node, succ.is_left);
          break;

        case 1:
          bool child_is_left {node_at(toDelete_idx).left != nullindex};
          index_type child_idx {child_is_left ? node_at(toDelete_idx).left : node_at(toDelete_idx).right};

          if (parent_idx == nullindex) {
            root = child_idx;
            node_at(child_idx).parent = nullindex;
            set_color(child_idx, Color::Black);
            delete_node(toDelete_idx);
            return;
          }

          node_at(child_idx).parent = parent_idx;
          set_color(child_idx, Color::Black);
          if (is_left_toDelete_for_parent) {
            node_at(parent_idx).left = child_idx;
          } else {
            node_at(parent_idx).right = child_idx;
          }
          delete_node(toDelete_idx);

          break;

        case 0:
          if (parent_idx == nullindex) {
            delete_node(toDelete_idx);
            root = nullindex;
            return;
          }

          if (is_left_toDelete_for_parent) {
            node_at(parent_idx).left = nullindex;
          } else {
            node_at(parent_idx).right = nullindex;
          }
          Color colorToDelete {color(toDelete_idx)};
          delete_node(toDelete_idx);

          if (colorToDelete == Color::Black) {
            // FIXME: idx may be wrong
            balance_after_delete(toDelete_idx, is_left_toDelete_for_parent);
          }
      }
    }

    void balance_after_insert(index_type toDelete_idx);
    void balance_after_delete(index_type broken_idx, bool is_left_broken) {
      auto [parent_idx, std::ignore] {get_parent_and_sister(broken_idx)};

      switch (color(parent_idx)) {
        case Color::Red: balance_delete_red_parent_case(broken_idx, is_left_broken); break;
        case Color::Black: balance_delete_black_parent_case(broken_idx, is_left_broken);
      }
    }

    void balance_delete_red_parent_case(index_type broken_idx, bool is_left_broken) {
      auto [parent_idx, sister_idx] {get_parent_and_sister(broken_idx)};

      bool sister_has_red_child {node_at(sister_idx).left != nullindex || node_at(sister_idx).right != nullindex};

      if (sister_has_red_child) {
        bool is_left_child {node_at(sister_idx).left != nullindex};

        if (is_left_child)
          rotate_right(sister_idx);
        else
          rotate_left(sister_idx);

        if (is_left_broken)
          rotate_left(parent_idx);
        else
          rotate_right(parent_idx);

      } else
        set_color(sister_idx, Color::Red);

      set_color(parent_idx, Color::Black);
    }

    void balance_delete_black_parent_case(index_type broken_idx, bool is_left_broken) {
      auto [parent_idx, sister_idx] {get_parent_and_sister(broken_idx)};

      switch (color(sister_idx)) {
        case Color::Red: balance_delete_black_parent_red_sister_case(broken_idx, is_left_broken); break;
        case Color::Black: balance_delete_black_parent_black_sister_case(broken_idx, is_left_broken);
      }
    }

    void balance_delete_black_parent_red_sister_case(index_type broken_idx, bool is_left_broken) {
      auto [parent_idx, sister_idx] {get_parent_and_sister(broken_idx)};
      auto& res {search_red_node_in_childs_childs(sister_idx)};

      if (res.destination == nullindex) {
        if (is_left_broken)
          rotate_right(parent_idx);
        else
          rotate_left(parent_idx);

        return;
      }
    }

    void balance_delete_black_parent_black_sister_case(index_type broken_idx, bool is_left_broken);


    std::pair<index_type, index_type> get_parent_and_sister(index_type idx) {
      index_type parent {node_at(idx).parent};
      index_type sister {(node_at(parent).left == idx) ? node_at(parent).right : node_at(parent).left};
      return {parent, sister};
    }
    struct RedInBlackChilds_SearchRes {
      index_type destination;
      index_type parent_of_dest;
      bool is_left_dest;
      bool is_left_parent_of_dest;
    };
    RedInBlackChilds_SearchRes search_red_node_in_childs_childs(index_type root_idx) {
      auto [left_child, right_child] {get_childs(root_idx)};

      auto left_res {find_child_by_color(left_child, Color::Red)};
      auto right_res {find_child_by_color(right_child, Color::Red)};

      if (left_res.first != nullindex)
        return {left_res.first, left_child, left_res.second, true};
      else if (right_res.first != nullindex)
        return {right_res.first, right_child, right.second, false};
      else
        return {nullindex, nullindex, false, false};
    }

    std::pair<index_type, bool> find_child_by_color(index_type idx, Color color) {
      auto [left_child, right_child] {get_childs(idx)};

      if (color(left_child) == color)
        return {left_child, true};
      else if (color(right_child) == color)
        return {right_child, false};
      else
        return {nullindex, false};
    }

    std::pair<index_type, index_type> get_childs(index_type idx) { return {node_at(idx).left, node_at(idx).right}; }

    Color color(index_type idx) { return node_at(idx).color; }
    void set_color(index_type idx, Color color) { node_at(idx).color = color; }

    void rotate_left(index_type x_idx);
    void rotate_right(index_type y_idx);


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
    SearchResult res {search(std::forward<UK>(key))};

    if (res.node != nullindex) {
      node_at(res.node).value = std::forward<UT>(value);
      return;
    }

    index_type new_idx {new_node(std::forward<UK>(key), std::forward<UT>(value))};
    Node& new_node_ref {node_at(new_idx)};
    new_node_ref.parent = res.parent;

    if (res.parent == nullindex) {
      root = new_idx;
      node_at(root).color = Color::Black;
      return;

    } else if (res.is_left)
      node_at(res.parent).left = new_idx;
    else
      node_at(res.parent).right = new_idx;

    balance_after_insert(new_idx);
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
  void RedBlackBST<K, T>::balance_after_insert(index_type New_idx) {
    while (New_idx != root && node_at(node_at(New_idx).parent).color == Color::Red) {
      index_type parent_idx {node_at(New_idx).parent};
      index_type grandpa_idx {node_at(parent_idx).parent};

      bool parent_is_left {parent_idx == node_at(grandpa_idx).left};
      index_type uncle_idx {parent_is_left ? node_at(parent_idx).right : node_at(parent_idx).left};

      if (uncle_idx != nullindex && node_at(uncle_idx).color == Color::Red) {
        node_at(parent_idx).color = Color::Black;
        node_at(uncle_idx).color = Color::Black;
        node_at(grandpa_idx).color = Color::Red;
        New_idx = grandpa_idx;
        continue;
      }

      if ((parent_is_left && New_idx == node_at(parent_idx).right) ||
          (!parent_is_left && New_idx == node_at(parent_idx).left)) {
        if (parent_is_left)
          rotate_left(parent_idx);
        else
          rotate_right(parent_idx);
        std::swap(New_idx, parent_idx);
        grandpa_idx = node_at(New_idx).parent;
      }

      node_at(parent_idx).color = Color::Black;
      node_at(grandpa_idx).color = Color::Red;
      if (parent_is_left)
        rotate_right(grandpa_idx);
      else
        rotate_left(grandpa_idx);

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
