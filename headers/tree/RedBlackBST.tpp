#pragma once

#include <algorithm>
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

    struct Node {
      index_type parent, left, right;
      K key;
      T value;
      bool red;
    };

   public:
    using value_type = T;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

    RedBlackBST() : data {}, freeList {}, root {nullindex} {}
    RedBlackBST(const RedBlackBST&) = default;
    RedBlackBST& operator=(const RedBlackBST&) = default;
    RedBlackBST(RedBlackBST&&) = default;
    RedBlackBST& operator=(RedBlackBST&&) = default;
    ~RedBlackBST() { clear(); }


    void insert(const K& key) { insert_impl(key); }
    void insert(K&& key) { insert_impl(std::move(key)); }

    void remove(const K& key) { remove_impl(key); }
    void remove(K&& key) { remove_impl(std::move(key)); }


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


    template <typename UK, typename UT>
    void insert_impl(UK&& key, UT&& value) {
      if (root == nullindex) {
        root = new_node(std::forward(key), std::forward(value));
        return;
      }
      Node& node {node_at(root)};
      while (true) {
        if (value < node.value) {
          if (node.left == nullindex) {
            node.left = new_node(std::forward(key), std::forward(value));
            return;
          }
          node = node.left;
        } else {
          if (node.right == nullindex) {
            node.right = new_node(std::forward(key), std::forward(value));
            return;
          }
          node = node.right;
        }
      }
    }

    template <typename UK, typename UT>
    void remove_impl(UK&& key, UT&& value) {}


    void shrink_to_fit() {
      std::vector<std::pair<K, T>> elements;
      elements.reserve(size());
      extract_in_order(root, elements);

      data.clear();
      freeList = {};

      root = build_balanced(elements, 0, elements.size() - 1, nullindex);
    }

    void extract_in_order(index_type idx, std::vector<std::pair<K, T>>& out) {
      if (idx == nullindex) return;
      Node& node {node_at(idx)};

      extract_in_order(node.left, out);

      out.emplace_back(std::move(node.key), std::move(node.value));
      delete_node(idx);

      extract_in_order(node.right, out);
    }

    struct BuildResult {
      index_type node;
      size_t height;
    };
    BuildResult build_balanced(const std::vector<std::pair<K, T>>& elements, size_t start, size_t end,
                               index_type parent) {
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

      node.red = (height % 2 == 1);

      return {idx, height};
    }

    const Node& node_at(index_type idx) const { return *std::launder(reinterpret_cast<const Node*>(&data[idx])); }
    Node& node_at(index_type idx) { return const_cast<Node&>(std::as_const(*this).node_at(idx)); }

    template <typename UK, typename UT>
    index_type new_node(UK&& key, UT&& value) {
      index_type idx;
      if (!freeList.empty()) {
        idx = freeList.top();
        freeList.pop();
      } else {
        idx = static_cast<index_type>(data.size());
        data.emplace_back();
      }
      new (&data[idx]) Node {nullindex, nullindex, nullindex, std::forward<UK>(key), std::forward<UT>(value), true};

      return idx;
    }

    void delete_node(index_type idx) {
      node_at(idx)->~Node();
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

}
