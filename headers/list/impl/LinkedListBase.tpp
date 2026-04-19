#pragma once


#include "impl/nodeTypes.tpp"

#include "impl/nodeAllocator.tpp"


namespace Base {

  template <typename T, typename Derived, typename NodeType, typename A = std::allocator<T>>
  class LinkedListBase {

   protected:
    using NodeAlloc = Allocator::NodeAllocator<NodeType, A>;
    NodeAlloc alloc;
    NodeType* head;
    NodeType* tail;
    size_t len;


    NodeType* allocate_node(const T& value) {
      NodeType* node = alloc.allocate();
      NodeType::construct(node, value);
      return node;
    }
    void deallocate_node(NodeType* node) {
      NodeType::destroy(node);
      alloc.deallocate(node);
    }


   public:
    template <bool IsConst>
    class Iterator {
     private:
      using NodePtr = std::conditional_t<IsConst, const NodeType*, NodeType*>;
      NodePtr node;

     public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;
      using difference_type = std::ptrdiff_t;
      using pointer = std::conditional_t<IsConst, const T*, T*>;
      using reference = std::conditional_t<IsConst, const T&, T&>;

      Iterator() = default;
      explicit Iterator(NodePtr ptr = nullptr) : node(ptr) {}

      template <bool OtherConst, typename = std::enable_if_t<IsConst && !OtherConst>>
      Iterator(const Iterator<OtherConst>& other) : node(other.node) {}

      reference operator*() const { return NodeType::value(*node); }
      pointer operator->() const { return &NodeType::value(*node); }

      Iterator& operator++() {
        node = NodeType::next(*node);
        return *this;
      }
      Iterator operator++(int) {
        Iterator tmp = *this;
        node = NodeType::next(*node);
        return tmp;
      }

      friend bool operator==(const Iterator& a, const Iterator& b) { return a.node == b.node; }
      friend bool operator!=(const Iterator& a, const Iterator& b) { return a.node != b.node; }

      NodeType* raw() const { return const_cast<NodeType*>(node); }
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    iterator begin() { return iterator {head}; }
    iterator end() { return iterator {nullptr}; }
    const_iterator begin() const { return const_iterator {head}; }
    const_iterator end() const { return const_iterator {nullptr}; }


    explicit LinkedListBase(const A& allocator = A()) : alloc {allocator}, head {nullptr}, tail {nullptr}, len {} {}

    LinkedListBase(const LinkedListBase&) = delete;
    LinkedListBase& operator=(const LinkedListBase&) = delete;

    LinkedListBase(LinkedListBase&& other) noexcept
        : alloc {std::move(other.alloc)}, head {other.head}, tail {other.tail}, len {other.len} {
      other.head = nullptr;
      other.tail = nullptr;
      other.len = 0;
    }
    LinkedListBase& operator=(LinkedListBase&& other) noexcept {
      if (this != &other) {
        clear();
        alloc = std::move(other.alloc);
        head = other.head;
        tail = other.tail;
        len = other.len;
        other.head = nullptr;
        other.tail = nullptr;
        other.len = 0;
      }
      return *this;
    }
    virtual ~LinkedListBase() { clear(); }

    void clear() {
      NodeType* cur {head};
      while (cur) {
        NodeType* next {NodeType::next(*cur)};
        deallocate_node(cur);
        cur = next;
      }
      head = nullptr;
      tail = nullptr;
      len = 0;
    }


    size_t size() const { return len; }
    bool empty() const { return len == 0; }


    T& front() { return NodeType::value(*head); }
    const T& front() const { return NodeType::value(*head); }
    T& back() { return NodeType::value(*tail); }
    const T& back() const { return NodeType::value(*tail); }

    void push_front(const T& value) {
      NodeType* node {allocate_node(value)};
      NodeType::set_next(*node, head);
      head = node;
      if (!tail) tail = node;
      ++len;
      static_cast<Derived*>(this)->after_push_front(node);
    }

    void pop_front() {
      NodeType* node {head};
      head = NodeType::next(*head);
      if (!head) tail = nullptr;
      static_cast<Derived*>(this)->before_pop_front(node);
      deallocate_node(node);
      --len;
    }

    void after_push_front(NodeType*) {}
    void before_pop_front(NodeType*) {}


    virtual void push_back(const T& value) = 0;
    virtual void pop_back() = 0;
  };

}
