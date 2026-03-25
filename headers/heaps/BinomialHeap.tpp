#pragma once

#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

namespace aoi {

  template <typename T, typename P = int32_t, typename Compare = std::less<P>, typename A = std::allocator<T>>
  class BinomialHeap {
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
      Node* sibling;
      size_t degree;

      Node(NodeData* data) : data {data}, parent {nullptr}, child {nullptr}, sibling {nullptr}, degree {0} {}
    };

    using NodeDataAllocator = typename std::allocator_traits<A>::template rebind_alloc<NodeData>;
    using NodeAllocator = typename std::allocator_traits<A>::template rebind_alloc<Node>;


   public:
    explicit BinomialHeap(const Compare& comp = Compare(), const NodeAllocator& alloc = NodeAllocator(),
                          const NodeDataAllocator& dataAlloc = NodeDataAllocator())
        : head {nullptr}, comp {comp}, alloc {alloc}, dataAlloc {dataAlloc} {}

    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;

    BinomialHeap(BinomialHeap&& other) noexcept
        : head {other.head}, comp {std::move(other.comp)}, alloc {std::move(other.alloc)},
          dataAlloc {std::move(other.dataAlloc)} {
      other.head = nullptr;
    }
    BinomialHeap& operator=(BinomialHeap&& other) noexcept {
      if (this != &other) {
        clear();
        head = other.head;
        comp = std::move(other.comp);
        alloc = std::move(other.alloc);
        dataAlloc = std::move(other.dataAlloc);
        other.head = nullptr;
      }
      return *this;
    }
    ~BinomialHeap() { clear(); }


    std::pair<const T&, const P&> peekWithPriority() const {
      if (isEmpty()) {
        throw std::runtime_error("BinomialHeap::peekWithPriority(): heap is empty");
      }
      Node* minNode = head;
      Node* current = head;
      while (current) {
        if (higherPriority(comp, current, minNode)) {
          minNode = current;
        }
        current = current->sibling;
      }
      return {minNode->data->value, minNode->data->priority};
    }
    const T& peek() const { return peekWithPriority().first; }

    NodeData* push(P priority, const T& value) { return push_impl(priority, value); }
    NodeData* push(P priority, T&& value) { return push_impl(priority, std::move(value)); }

    void pop();

    void decreasePriority(NodeData* nodeData, P newPriority) {
      nodeData->priority = newPriority;
      heapUp(nodeData->node);
    }

    void merge(BinomialHeap& other);

    void delete_key(NodeData* node) {
      if (isEmpty()) {
        throw std::runtime_error("BinomialHeap::delete_key(): heap is empty");
      }
      P extremePriority = comp(std::numeric_limits<P>::max(), std::numeric_limits<P>::lowest())
                              ? std::numeric_limits<P>::max()
                              : std::numeric_limits<P>::lowest();
      decreasePriority(node, extremePriority);
      pop();
    }

    bool isEmpty() const { return head == nullptr; }


   private:
    Node* head;
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

    void heapUp(Node* node) {
      while (node->parent and higherPriority(comp, node, node->parent)) {
        swapNodes(node, node->parent);
        node = node->parent;
      }
    }

    static Node* linkTwoTrees(const Compare& comp, Node* a, Node* b) {
      if (higherPriority(comp, b, a)) {
        std::swap(a, b);
      }
      return linkTrees(a, b);
    }

    static Node* linkTrees(Node* parent, Node* child) {
      child->parent = parent;
      child->sibling = parent->child;
      parent->child = child;
      parent->degree++;
      return parent;
    }

    static Node* reverseChilds(Node* node) {
      Node* prev {nullptr};
      while (node) {
        Node* next = node->sibling;
        node->parent = nullptr;
        node->sibling = prev;
        prev = node;
        node = next;
      }
      return prev;
    }

    template <typename U>
    NodeData* push_impl(P priority, U&& value) {
      NodeData* new_data = std::allocator_traits<NodeDataAllocator>::allocate(dataAlloc, 1);
      std::allocator_traits<NodeDataAllocator>::construct(dataAlloc, new_data, priority, std::forward<U>(value));

      Node* new_node = std::allocator_traits<NodeAllocator>::allocate(alloc, 1);
      std::allocator_traits<NodeAllocator>::construct(alloc, new_node, new_data);

      new_data->node = new_node;

      BinomialHeap tmp {comp, alloc, dataAlloc};
      tmp.head = new_node;
      merge(tmp);
      return new_data;
    }

    void clear() noexcept {
      std::stack<Node*> clearStack;
      if (head) {
        clearStack.push(head);
      }
      while (!clearStack.empty()) {
        Node* current = clearStack.top();
        clearStack.pop();
        if (current->child) clearStack.push(current->child);
        if (current->sibling) clearStack.push(current->sibling);

        std::allocator_traits<NodeDataAllocator>::destroy(dataAlloc, current->data);
        std::allocator_traits<NodeDataAllocator>::deallocate(dataAlloc, current->data, 1);

        std::allocator_traits<NodeAllocator>::destroy(alloc, current);
        std::allocator_traits<NodeAllocator>::deallocate(alloc, current, 1);
      }
      head = nullptr;
    }
  };


  template <typename T, typename P, typename Compare, typename A>
  void BinomialHeap<T, P, Compare, A>::pop() {
    if (isEmpty()) {
      throw std::runtime_error("BinomialHeap::pop(): heap is empty");
    }
    Node* minNode = head;
    Node* current = head;
    while (current) {
      if (higherPriority(comp, current, minNode)) {
        minNode = current;
      }
      current = current->sibling;
    }
    Node* childs {minNode->child};
    Node* prev {nullptr};
    Node* iter {head};
    while (iter and iter != minNode) {
      prev = iter;
      iter = iter->sibling;
    }
    if (prev) {
      prev->sibling = minNode->sibling;
    } else {
      head = minNode->sibling;
    }

    BinomialHeap childHeap {comp, alloc, dataAlloc};
    childHeap.head = reverseChilds(childs);
    merge(childHeap);

    std::allocator_traits<NodeDataAllocator>::destroy(dataAlloc, minNode->data);
    std::allocator_traits<NodeDataAllocator>::deallocate(dataAlloc, minNode->data, 1);
    std::allocator_traits<NodeAllocator>::destroy(alloc, minNode);
    std::allocator_traits<NodeAllocator>::deallocate(alloc, minNode, 1);
  }

  template <typename T, typename P, typename Compare, typename A>
  void BinomialHeap<T, P, Compare, A>::merge(BinomialHeap& other) {
    if (this == &other) return;

    BinomialHeap merged {comp, alloc, dataAlloc};

    Node* carry {nullptr};
    Node* tail {nullptr};
    Node *t1 {head}, *t2 {other.head};
    for (size_t i = 0; t1 or t2 or carry; ++i) {
      Node* t1_i {t1 and t1->degree == i ? t1 : nullptr};
      Node* t2_i {t2 and t2->degree == i ? t2 : nullptr};
      Node* next_t1 {t1_i ? t1->sibling : t1};
      Node* next_t2 {t2_i ? t2->sibling : t2};
      Node* r_i {nullptr};

      if (t1_i and t2_i and carry) {
        r_i = carry;
        carry = linkTwoTrees(comp, t1_i, t2_i);
      } else if (t1_i and t2_i) {
        carry = linkTwoTrees(comp, t1_i, t2_i);
        r_i = nullptr;
      } else if (t1_i and carry) {
        carry = linkTwoTrees(comp, t1_i, carry);
        r_i = nullptr;
      } else if (t2_i and carry) {
        carry = linkTwoTrees(comp, t2_i, carry);
        r_i = nullptr;
      } else if (t1_i) {
        r_i = t1_i;
      } else if (t2_i) {
        r_i = t2_i;
      } else if (carry) {
        r_i = carry;
        carry = nullptr;
      }
      if (r_i) {
        if (!merged.head) {
          merged.head = r_i;
          tail = r_i;
        } else {
          tail->sibling = r_i;
          tail = r_i;
        }
        r_i->sibling = nullptr;
      }
      t1 = next_t1;
      t2 = next_t2;
    }
    head = nullptr;
    other.head = nullptr;
    head = merged.head;
    merged.head = nullptr;
  }

}
