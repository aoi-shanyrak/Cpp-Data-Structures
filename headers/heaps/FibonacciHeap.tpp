#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace aoi {

  template <typename T, typename P = int32_t, typename Compare = std::less<P>, typename A = std::allocator<T>>
  class FibonacciHeap {
    struct Node;

    struct NodeData {
      P priority;
      T value;
      Node* node;

      NodeData(P p, T v) : priority {p}, value {v}, node {nullptr} {}
    };
    struct Node {
      NodeData* data;
      Node* parent;
      Node* child;
      Node* left;
      Node* right;
      size_t degree;
      bool marked;

      Node(NodeData* data)
          : data {data}, parent {nullptr}, child {nullptr}, left {this}, right {this}, degree {0}, marked {false} {}
    };

    using NodeDataAllocator = typename std::allocator_traits<A>::template rebind_alloc<NodeData>;
    using NodeAllocator = typename std::allocator_traits<A>::template rebind_alloc<Node>;

   public:
    using value_type = T;
    using priority_type = P;
    using compare_type = Compare;
    using allocator_type = A;
    using Handle = NodeData*;

    explicit FibonacciHeap(const Compare& comp = Compare(), const NodeAllocator& alloc = NodeAllocator(),
                           const NodeDataAllocator& dataAlloc = NodeDataAllocator())
        : head {nullptr}, min {nullptr}, comp {comp}, alloc {alloc}, dataAlloc {dataAlloc} {}

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : head {other.head}, min {other.min}, comp {std::move(other.comp)}, alloc {std::move(other.alloc)},
          dataAlloc {std::move(other.dataAlloc)} {
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
        dataAlloc = std::move(other.dataAlloc);
        other.head = nullptr;
        other.min = nullptr;
      }
      return *this;
    }
    ~FibonacciHeap() { clear(); }


    std::pair<const T&, const P&> peekWithPriority() const {
      if (empty()) throw std::runtime_error("FibonacciHeap::peekWithPriority(): heap is empty");
      return {min->data->value, min->data->priority};
    }
    const T& peek() const { return peekWithPriority().first; }

    NodeData* push(P priority, const T& value) { return push_impl(priority, value); }
    NodeData* push(P priority, T&& value) { return push_impl(priority, std::move(value)); }

    void pop();

    void decreasePriority(NodeData* nodeData, P newPriority);

    void merge(FibonacciHeap& other) {
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

    void delete_key(NodeData* nodeData);

    bool empty() const noexcept { return head == nullptr; }


   private:
    Node* head;
    Node* min;
    Compare comp;
    NodeAllocator alloc;
    NodeDataAllocator dataAlloc;


    static bool higherPriority(const Compare& comp, Node* a, Node* b) {
      return comp(a->data->priority, b->data->priority);
    }

    static void swapNodes(Node* a, Node* b) {
      std::swap(a->data, b->data);
      std::swap(a->data->node, b->data->node);
    }

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

    void consolidate();

    template <typename U>
    NodeData* push_impl(P priority, U&& value) {
      auto [new_node, new_data] = create_node(priority, std::forward<U>(value));

      new_data->node = new_node;

      FibonacciHeap tmp {comp, alloc, dataAlloc};
      tmp.head = new_node;
      tmp.min = new_node;
      merge(tmp);

      return new_data;
    }


    void clear() noexcept;

    template <typename U>
    std::pair<Node*, NodeData*> create_node(P priority, U&& value) {
      NodeData* new_data = std::allocator_traits<NodeDataAllocator>::allocate(dataAlloc, 1);
      std::allocator_traits<NodeDataAllocator>::construct(dataAlloc, new_data, priority, std::forward<U>(value));

      Node* new_node = std::allocator_traits<NodeAllocator>::allocate(alloc, 1);
      std::allocator_traits<NodeAllocator>::construct(alloc, new_node, new_data);

      return {new_node, new_data};
    }

    void delete_node(Node* node) {
      std::allocator_traits<NodeDataAllocator>::destroy(dataAlloc, node->data);
      std::allocator_traits<NodeDataAllocator>::deallocate(dataAlloc, node->data, 1);
      std::allocator_traits<NodeAllocator>::destroy(alloc, node);
      std::allocator_traits<NodeAllocator>::deallocate(alloc, node, 1);
    }
  };


  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::pop() {
    if (empty()) {
      throw std::runtime_error("FibonacciHeap::pop(): heap is empty");
    }
    if (min->right == min) {
      head = nullptr;
    } else {
      min->left->right = min->right;
      min->right->left = min->left;
      if (head == min) {
        head = min->right;
      }
    }

    if (min->child) {
      FibonacciHeap childHeap {comp, alloc, dataAlloc};
      childHeap.head = min->child;
      childHeap.min = min->child;

      Node* current {min->child};
      do {
        current->parent = nullptr;
        if (higherPriority(comp, current, childHeap.min)) {
          childHeap.min = current;
        }
        current = current->right;
      } while (current != min->child);

      merge(childHeap);
    }

    delete_node(min);

    if (head) {
      consolidate();
    } else {
      min = nullptr;
    }
  }

  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::consolidate() {
    std::vector<Node*> degreeTable;
    Node* current {head};
    do {
      Node* next {current->right};
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
      current = next;

    } while (current != head);

    head = min = nullptr;
    for (Node* node : degreeTable) {
      if (node) {
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
  }

}
