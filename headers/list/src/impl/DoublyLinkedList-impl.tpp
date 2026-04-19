#pragma once


template <typename T, typename A = std::allocator<T>>
class DoublyLinkedList : public Base::LinkedList_Base<T> {
 private:
  using Node = NodeType::DoublyNode<T>;
  using NodeAlloc = Allocator::NodeAllocator<Node, A>;

  NodeAlloc alloc;

 public:
  template <bool IsConst>
  class Iterator {
   private:
    using NodePtr = std::conditional_t<IsConst, const Node*, Node*>;
    NodePtr node;

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;

    Iterator() = default;
    explicit Iterator(NodePtr ptr = nullptr) : node(ptr) {}

    template <bool OtherConst, typename = std::enable_if_t<IsConst && !OtherConst>>
    Iterator(const Iterator<OtherConst>& other) : node(other.node) {}

    reference operator*() const { return node->data; }
    pointer operator->() const { return &node->data; }

    Iterator& operator++() {
      node = static_cast<NodePtr>(node->next);
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      node = static_cast<NodePtr>(node->next);
      return tmp;
    }

    Iterator& operator--() {
      node = node->prev;
      return *this;
    }
    Iterator operator--(int) {
      Iterator tmp = *this;
      node = node->prev;
      return tmp;
    }

    friend bool operator==(const Iterator& a, const Iterator& b) { return a.node == b.node; }
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.node != b.node; }

    friend class DoublyLinkedList;
  };
  template <bool>
  friend class Iterator;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  explicit DoublyLinkedList(const A& alloc = A()) : Base::LinkedList_Base<T>(), alloc {alloc} {}

  iterator begin() { return iterator {static_cast<Node*>(this->head)}; }
  iterator end() { return iterator {nullptr}; }
  const_iterator begin() const { return const_iterator {static_cast<const Node*>(this->head)}; }
  const_iterator end() const { return const_iterator {nullptr}; }

  reverse_iterator rbegin() { return reverse_iterator {end()}; }
  reverse_iterator rend() { return reverse_iterator {begin()}; }
  const_reverse_iterator rbegin() const { return const_reverse_iterator {end()}; }
  const_reverse_iterator rend() const { return const_reverse_iterator {begin()}; }

  iterator insert(iterator pos, T value) {
    Node* new_node = alloc.allocate();
    new (new_node) Node {std::move(value)};

    if (pos.node == nullptr) {
      new_node->next = nullptr;
      new_node->prev = static_cast<Node*>(this->tail);
      if (this->tail) {
        static_cast<Node*>(this->tail)->next = new_node;
      }
      this->tail = new_node;
      if (!this->head) {
        this->head = new_node;
      }
    } else {
      Node* target = static_cast<Node*>(pos.node);
      new_node->next = target;
      new_node->prev = target->prev;

      if (target->prev) {
        static_cast<Node*>(target->prev)->next = new_node;
      } else {
        this->head = new_node;
      }
      target->prev = new_node;
    }

    ++this->len;
    return iterator {new_node};
  }

  iterator erase(iterator pos) {
    if (pos.node == nullptr) return pos;

    Node* node = static_cast<Node*>(pos.node);
    Node* next = static_cast<Node*>(node->next);

    if (node->prev) {
      static_cast<Node*>(node->prev)->next = node->next;
    } else {
      this->head = node->next;
    }

    if (node->next) {
      static_cast<Node*>(node->next)->prev = node->prev;
    } else {
      this->tail = node->prev;
    }

    node->~DoublyNode();
    alloc.deallocate(node);
    --this->len;

    return iterator {next};
  }

  void push_front(T value) override {
    Node* new_node = alloc.allocate();
    new (new_node) Node {std::move(value)};

    new_node->next = this->head;
    new_node->prev = nullptr;

    if (this->head) {
      static_cast<Node*>(this->head)->prev = new_node;
    }
    this->head = new_node;

    if (!this->tail) {
      this->tail = new_node;
    }
    ++this->len;
  }

  void push_back(T value) override {
    Node* new_node = alloc.allocate();
    new (new_node) Node {std::move(value)};

    new_node->next = nullptr;
    new_node->prev = static_cast<Node*>(this->tail);

    if (this->tail) {
      static_cast<Node*>(this->tail)->next = new_node;
    }
    this->tail = new_node;

    if (!this->head) {
      this->head = new_node;
    }
    ++this->len;
  }

  void pop_front() override {
    if (this->empty()) throw std::runtime_error("DoublyLinkedList is empty");

    Node* node = static_cast<Node*>(this->head);
    this->head = node->next;

    if (this->head) {
      static_cast<Node*>(this->head)->prev = nullptr;
    } else {
      this->tail = nullptr;
    }

    node->~DoublyNode();
    alloc.deallocate(node);
    --this->len;
  }

  void pop_back() override {
    if (this->empty()) throw std::runtime_error("DoublyLinkedList is empty");

    Node* node = static_cast<Node*>(this->tail);
    this->tail = node->prev;

    if (this->tail) {
      static_cast<Node*>(this->tail)->next = nullptr;
    } else {
      this->head = nullptr;
    }

    node->~DoublyNode();
    alloc.deallocate(node);
    --this->len;
  }

  T& front() override { return static_cast<Node*>(this->head)->data; }
  T& back() override { return static_cast<Node*>(this->tail)->data; }
  const T& front() const override { return static_cast<const Node*>(this->head)->data; }
  const T& back() const override { return static_cast<const Node*>(this->tail)->data; }

  void clear() override {
    Node* current = static_cast<Node*>(this->head);
    while (current) {
      Node* next = static_cast<Node*>(current->next);
      current->~DoublyNode();
      alloc.deallocate(current);
      current = next;
    }
    this->head = this->tail = nullptr;
    this->len = 0;
  }

  ~DoublyLinkedList() { clear(); }
};
