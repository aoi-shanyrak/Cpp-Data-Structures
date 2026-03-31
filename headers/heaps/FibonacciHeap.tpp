#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

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
          : data {data}, parent {nullptr}, child {nullptr}, left {nullptr}, right {nullptr}, degree {0},
            marked {false} {}
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
        : head {nullptr}, tail {nullptr}, min {nullptr}, comp {comp}, alloc {alloc}, dataAlloc {dataAlloc} {}

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : head {other.head}, tail {other.tail}, min {other.min}, comp {std::move(other.comp)},
          alloc {std::move(other.alloc)}, dataAlloc {std::move(other.dataAlloc)} {
      other.head = nullptr;
      other.tail = nullptr;
      other.min = nullptr;
    }
    FibonacciHeap& operator=(FibonacciHeap&& other) noexcept {
      if (this != &other) {
        clear();
        head = other.head;
        tail = other.tail;
        min = other.min;
        comp = std::move(other.comp);
        alloc = std::move(other.alloc);
        dataAlloc = std::move(other.dataAlloc);
        other.head = nullptr;
      }
      return *this;
    }
    ~FibonacciHeap() { clear(); }


    std::pair<const T&, const P&> peekWithPriority() const {
      if (empty()) throw std::runtime_error("FibonacciHeap::peekWithPriority(): heap is empty");
      return {minNode->data->value, minNode->data->priority};
    }
    const T& peek() const { return peekWithPriority().first; }

    NodeData* push(P priority, const T& value) { return push_impl(priority, value); }
    NodeData* push(P priority, T&& value) { return push_impl(priority, std::move(value)); }

    void pop();

    void decreasePriority(NodeData* nodeData, P newPriority);

    void merge(FibonacciHeap& other) {
      tail->right = other.head;
      other.head->left = tail;
      tail = other.tail;
      other.head = nullptr;
      other.tail = nullptr;

      if (higherPriority(comp, other.minNode, min)) {
        min = other.minNode;
      }
      other.minNode = nullptr;
    }

    void delete_key(NodeData* nodeData);

    bool empty() const noexcept { return head == nullptr; }


   private:
    Node *head, *tail, *min;
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

    void consolidate();

    template <typename U>
    NodeData* push_impl(P priority, U&& value) {
      auto [new_node, new_data] = create_node(priority, std::forward<U>(value));

      new_data->node = new_node;

      FibonacciHeap tmp {comp, alloc, dataAlloc};
      tmp.head = new_node;
      tmp.tail = new_node;
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
    Node* childsHead {minNode->child};
    Node* childsTail {childsHead};
    Node* childsMin {childsHead};

    if (childsHead) {
      Node* current = childsHead;
      while (true) {
        if (higherPriority(comp, current, childsMin)) {
          childsMin = current;
        }
        current->parent = nullptr;
        if (current->right) {
          current = current->right;
        } else
          break;
      }
      childsTail = current;
    }

    if (!minNode->right) {
      tail = minNode->left;
    } else if (!minNode->left) {
      head = minNode->right;
    } else {
      minNode->left = minNode->right;
    }

    if (childsHead) {
      FibonacciHeap tmp {comp, alloc, dataAlloc};
      tmp.head = childsHead;
      tmp.tail = childsTail;
      tmp.min = childsMin;
      merge(tmp);
    }
  }

  template <typename T, typename P, typename Compare, typename A>
  void FibonacciHeap<T, P, Compare, A>::consolidate() {
  }

}
