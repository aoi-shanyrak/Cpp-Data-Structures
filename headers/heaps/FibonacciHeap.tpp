#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

namespace aoi {

  template <typename T, typename P = int32_t, typename Compare = std::less<P>, typename A = std::allocator<T>>
  class FibonacciHeap {
    struct Node {
      P priority;
      T value;
      Node* parent;
      Node* child;
      Node* left;
      Node* right;
      size_t degree;
      bool marked;

      Node(P p, T v)
          : priority {p}, value {v}, parent {nullptr}, child {nullptr}, left {this}, right {this}, degree {0},
            marked {false} {}
    };
    using NodeAllocator = typename std::allocator_traits<A>::template rebind_alloc<Node>;
    using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;


   public:
    using value_type = T;
    using priority_type = P;
    using compare_type = Compare;
    using allocator_type = A;
    using Handle = Node*;


    explicit FibonacciHeap(const Compare& comp = Compare(), const NodeAllocator& alloc = NodeAllocator())
        : head {nullptr}, min {nullptr}, comp {comp}, alloc {alloc} {}

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : head {other.head}, min {other.min}, comp {std::move(other.comp)}, alloc {std::move(other.alloc)} {
      other.head = nullptr;
      other.min = nullptr;
    }
    FibonacciHeap& operator=(FibonacciHeap&& other) noexcept {
      if (this != &other) {
        clear();
        head = other.head;
        min = other.min;
        comp = std::move(other.comp);
        alloc = std::move(other.alloc);
        other.head = nullptr;
        other.min = nullptr;
      }
      return *this;
    }
    ~FibonacciHeap() { clear(); }


    std::pair<const T&, const P&> peekWithPriority() const {
      if (empty()) throw std::runtime_error("FibonacciHeap::peekWithPriority(): heap is empty");
      return {min->value, min->priority};
    }
    const T& peek() const { return peekWithPriority().first; }

    Node* push(P priority, const T& value) { return push_impl(priority, value); }
    Node* push(P priority, T&& value) { return push_impl(priority, std::move(value)); }

    void pop();

    void decreasePriority(Node* node, P newPriority) {
      if (!node) {
        throw std::invalid_argument("FibonacciHeap::decreasePriority(): node is null");
      }
      decreasePriority_impl(node, newPriority);
    }

    void merge(FibonacciHeap& other);

    void deleteKey(Node* node) {
      if (!node) {
        throw std::invalid_argument("FibonacciHeap::deleteKey(): node is null");
      }
      decreasePriority_impl(node, node->priority, true);
      pop();
    }

    bool empty() const noexcept { return head == nullptr; }


   private:
    Node* head;
    Node* min;
    Compare comp;
    NodeAllocator alloc;


    void consolidate();

    void clear() noexcept;

    static bool higherPriority(const Compare& comp, Node* a, Node* b) { return comp(a->priority, b->priority); }

    static Node* linkTwoTrees(const Compare& comp, Node* a, Node* b) {
      if (higherPriority(comp, b, a)) {
        std::swap(a, b);
      }
      return linkTrees(a, b);
    }

    static Node* linkTrees(Node* parent, Node* child) {
      child->parent = parent;
      if (!parent->child) {
        parent->child = child;
        child->left = child->right = child;
      } else {
        child->right = parent->child;
        child->left = parent->child->left;
        parent->child->left->right = child;
        parent->child->left = child;
      }
      parent->degree++;
      return parent;
    }

    void removeFromRootList(Node* node) {
      if (node->right == node) {
        head = nullptr;
      } else {
        node->left->right = node->right;
        node->right->left = node->left;
        if (head == node) {
          head = node->right;
        }
      }
    }

    void promoteChildsToRootList(Node* node) {
      FibonacciHeap childHeap {comp, alloc};
      childHeap.head = node->child;
      childHeap.min = node->child;

      Node* current {node->child};
      do {
        current->parent = nullptr;
        if (higherPriority(comp, current, childHeap.min)) {
          childHeap.min = current;
        }
        current = current->right;
      } while (current != node->child);

      merge(childHeap);
    }

    void cut(Node* node, Node* parent);

    void cascadingCut(Node* node) {
      if (!node) return;
      Node* parent = node->parent;
      if (parent) {
        if (!node->marked) {
          node->marked = true;
        } else {
          cut(node, parent);
          cascadingCut(parent);
        }
      }
    }

    template <typename U>
    Node* push_impl(P priority, U&& value) {
      Node* new_node = create_node(priority, std::forward<U>(value));

      FibonacciHeap tmp {comp, alloc};
      tmp.head = new_node;
      tmp.min = new_node;
      merge(tmp);

      return new_node;
    }

    void decreasePriority_impl(Node* node, P newPriority, bool force = false);


    template <typename U>
    Node* create_node(P priority, U&& value) {
      Node* new_node = NodeAllocatorTraits::allocate(alloc, 1);
      NodeAllocatorTraits::construct(alloc, new_node, priority, std::forward<U>(value));
      return new_node;
    }

    void delete_node(Node* node) {
      NodeAllocatorTraits::destroy(alloc, node);
      NodeAllocatorTraits::deallocate(alloc, node, 1);
    }
  };


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::pop() {
    if (empty()) {
      throw std::runtime_error("FibonacciHeap::pop(): heap is empty");
    }
    Node* oldMin = min;
    removeFromRootList(min);
    if (min->child) {
      promoteChildsToRootList(min);
    }
    if (head) {
      consolidate();
    } else {
      min = nullptr;
    }
    delete_node(oldMin);
  }


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::merge(FibonacciHeap& other) {
    if (this == &other) return;
    if (!other.head) return;
    if (!head) {
      head = other.head;
      min = other.min;
      other.head = other.min = nullptr;
      return;
    }

    Node* thisTail = head->left;
    Node* otherTail = other.head->left;

    thisTail->right = other.head;
    other.head->left = thisTail;
    otherTail->right = head;
    head->left = otherTail;

    if (higherPriority(comp, other.min, min)) {
      min = other.min;
    }
    other.head = other.min = nullptr;
  }


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::decreasePriority_impl(Node* node, P newPriority, bool force) {
    if (!force && comp(node->priority, newPriority)) {
      throw std::invalid_argument(
          "FibonacciHeap::decreasePriority_impl(): new priority is greater than current priority");
    }
    node->priority = newPriority;

    Node* parent {node->parent};
    if (parent && (higherPriority(comp, node, parent) or force)) {
      cut(node, parent);
      cascadingCut(parent);
    }
    if (higherPriority(comp, node, min) or force) {
      min = node;
    }
  }


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::consolidate() {
    if (!head) return;

    std::vector<Node*> roots;
    Node* current {head};
    do {
      roots.push_back(current);
      current = current->right;
    } while (current != head);

    std::vector<Node*> degreeTable;
    for (Node* current : roots) {
      size_t degree {current->degree};

      if (degree >= degreeTable.size()) {
        degreeTable.resize(degree + 1, nullptr);
      }
      while (degreeTable[degree]) {
        current = linkTwoTrees(comp, current, degreeTable[degree]);
        degreeTable[degree] = nullptr;
        degree++;
        if (degree >= degreeTable.size()) {
          degreeTable.resize(degree + 1, nullptr);
        }
      }
      degreeTable[degree] = current;
    }

    head = min = nullptr;
    for (Node* node : degreeTable) {
      if (!node) continue;
      node->left = node->right = node;
      if (!head) {
        head = min = node;
      } else {
        Node* tail {head->left};

        tail->right = node;
        node->left = tail;
        node->right = head;
        head->left = node;

        if (higherPriority(comp, node, min)) {
          min = node;
        }
      }
    }
  }


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::cut(Node* node, Node* parent) {
    if (node->right == node) {
      parent->child = nullptr;
    } else {
      node->left->right = node->right;
      node->right->left = node->left;
      if (parent->child == node) {
        parent->child = node->right;
      }
    }
    parent->degree--;

    if (!head) {
      head = node;
      min = node;
      node->left = node->right = node;
    } else {
      Node* tail {head->left};

      tail->right = node;
      node->left = tail;
      node->right = head;
      head->left = node;

      if (higherPriority(comp, node, min)) {
        min = node;
      }
    }
    node->parent = nullptr;
    node->marked = false;
  }


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::clear() noexcept {
    if (!head) return;

    std::stack<Node*> clearStack;
    Node* current {head};
    do {
      clearStack.push(current);
      current = current->right;
    } while (current != head);

    while (!clearStack.empty()) {
      Node* node = clearStack.top();
      clearStack.pop();

      if (node->child) {
        Node* child = node->child;
        do {
          clearStack.push(child);
          child = child->right;
        } while (child != node->child);
      }

      delete_node(node);
    }
    head = min = nullptr;
  }

}
