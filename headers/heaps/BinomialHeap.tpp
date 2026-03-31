#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>
#include <utility>

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

    using NodeDataAllocatorTraits = std::allocator_traits<NodeDataAllocator>;
    using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;


   public:
    using value_type = T;
    using priority_type = P;
    using compare_type = Compare;
    using allocator_type = A;
    using Handle = NodeData*;

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
      if (empty()) {
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
      heapUp(node, true);
      pop();
    }

    bool empty() const { return head == nullptr; }


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

    void heapUp(Node* node, bool alwaysSwap = false) {
      while (node->parent and (higherPriority(comp, node, node->parent) or alwaysSwap)) {
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
      auto [new_node, new_data] = create_node(priority, std::forward<U>(value));

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

        delete_node(current);
      }
      head = nullptr;
    }

    template <typename U>
    std::pair<Node*, NodeData*> create_node(P priority, U&& value) {
      NodeData* new_data = NodeDataAllocatorTraits::allocate(dataAlloc, 1);
      NodeDataAllocatorTraits::construct(dataAlloc, new_data, priority, std::forward<U>(value));

      Node* new_node = NodeAllocatorTraits::allocate(alloc, 1);
      NodeAllocatorTraits::construct(alloc, new_node, new_data);

      return {new_node, new_data};
    }

    void delete_node(Node* node) {
      NodeDataAllocatorTraits::destroy(dataAlloc, node->data);
      NodeDataAllocatorTraits::deallocate(dataAlloc, node->data, 1);
      NodeAllocatorTraits::destroy(alloc, node);
      NodeAllocatorTraits::deallocate(alloc, node, 1);
    }
  };


  template <typename T, typename P, typename Compare, typename A>
  void BinomialHeap<T, P, Compare, A>::pop() {
    if (empty()) {
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

    delete_node(minNode);
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
