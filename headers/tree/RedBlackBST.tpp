#pragma once

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

    RedBlackBST() = default;
    RedBlackBST(const RedBlackBST&) = default;
    RedBlackBST& operator=(const RedBlackBST&) = default;
    RedBlackBST(RedBlackBST&&) = default;
    RedBlackBST& operator=(RedBlackBST&&) = default;
    ~RedBlackBST() { clear(); }


    void insert(const K& key) { insert(key); }
    void insert(K&& key) { insert(std::move(key)); }

    void remove(const K& key) { remove(key); }
    void remove(K&& key) { remove(std::move(key)); }


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


    template <typename U>
    void insert(U&& key) {}

    template <typename U>
    void remove(U&& key) {}


    void shrink_to_fit() {
      std::vector<std::pair<K, T>> elements;
      elements.reserve(size());
      extract_in_order(root, elements);

      data.clear();
      freeList = {};

      root = build_balanced(elements, 0, elements.size() - 1);
    }

    void extract_in_order(index_type node, std::vector<std::pair<K, T>> out) {
      if (node == nullindex) return;
      extract_in_order(data[node].left, out);

      out.emplace_back(std::move(data[node].key), std::move(data[node].value));
      delete_node(node);

      extract_in_order(data[node].right, out);
    }

    template <typename UK, typename UT>
    Node& new_node(UK&& key, UT&& value) {
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
      Node* node = reinterpret_cast<Node*>(&data[idx]);
      node->~Node();
      freeList.push(idx);
    }

    void clear_subtree(index_type node) noexcept {
      if (node == nullindex) return;
      clear_subtree(data[node].left);
      clear_subtree(data[node].right);
      delete_node(node);
    }
  };

}
