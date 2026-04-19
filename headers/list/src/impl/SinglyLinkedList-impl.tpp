#pragma once


template <typename T, typename A = std::allocator<T>>
class SinglyLinkedList : public Base::LinkedList_Base<T> {
 private:
  using Node = NodeType::SinglyNode<T>;
  using NodeAlloc = Allocator::NodeAllocator<Node, A>;

  NodeAlloc alloc;

 public:
  template <bool IsConst>
  class Iterator {
   private:
    using NodePtr = std::conditional_t<IsConst, const Node*, Node*>;
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

    reference operator*() const { return node->data; }
    pointer operator->() const { return &node->data; }

    Iterator& operator++() {
      node = node->next;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      node = node->next;
      return tmp;
    }

    friend bool operator==(const Iterator& a, const Iterator& b) { return a.node == b.node; }
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.node != b.node; }

    friend class SinglyLinkedList;
  };
  template <bool>
  friend class Iterator;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  explicit SinglyLinkedList(const A& alloc = A()) : Base::LinkedList_Base<T>(), alloc {alloc} {}

  iterator begin() { return iterator {this->head}; }
  iterator end() { return iterator {nullptr}; }
  const_iterator begin() const { return const_iterator {this->head}; }
  const_iterator end() const { return const_iterator {nullptr}; }

  iterator insert_after(iterator pos, T value) {
    if (pos.node == nullptr) return end();

    Node* new_node = alloc.allocate();
    new (new_node) Node {std::move(value)};

    Node* target = static_cast<Node*>(pos.node);
    new_node->next = target->next;
    target->next = new_node;

    if (!new_node->next) {
      this->tail = new_node;
    }

    ++this->len;
    return iterator {new_node};
  }

  iterator erase_after(iterator pos) {
    if (pos.node == nullptr) return end();

    Node* target = static_cast<Node*>(pos.node);
    if (!target->next) return end();

    Node* to_delete = target->next;
    target->next = to_delete->next;

    if (!target->next) {
      this->tail = target;
    }

    to_delete->~SinglyNode();
    alloc.deallocate(to_delete);
    --this->len;

    return iterator {target->next};
  }

  void push_front(T value) override {
    Node* new_node = alloc.allocate();
    new (new_node) Node {std::move(value)};

    new_node->next = this->head;
    this->head = new_node;

    if (!this->tail) {
      this->tail = new_node;
    }
    ++this->len;
  }
  void push_back(T value) override {
    Node* new_node = alloc.allocate();
    new (new_node) Node {std::move(value)};

    if (!this->head) {
      this->head = this->tail = new_node;
    } else {
      this->tail->next = new_node;
      this->tail = new_node;
    }
    ++this->len;
  }

  void pop_front() override {
    if (this->empty()) throw std::runtime_error("SinglyLinkedList is empty");

    Node* node = this->head;
    this->head = node->next;
    if (!this->head) {
      this->tail = nullptr;
    }
    node->~SinglyNode();
    alloc.deallocate(node);
    --this->len;
  }
  void pop_back() override {
    if (this->empty()) throw std::runtime_error("SinglyLinkedList is empty");
    if (this->len == 1) {
      pop_front();
      return;
    }
    Node* prev = this->head;
    while (prev->next != this->tail) {
      prev = prev->next;
    }
    this->tail->~SinglyNode();
    alloc.deallocate(this->tail);

    this->tail = prev;
    this->tail->next = nullptr;
    --this->len;
  }

  T& front() override { return this->head->data; }
  T& back() override { return this->tail->data; }
  const T& front() const override { return this->head->data; }
  const T& back() const override { return this->tail->data; }

  void clear() override {
    Node* current = this->head;
    while (current) {
      Node* next = current->next;
      current->~SinglyNode();
      alloc.deallocate(current);
      current = next;
    }
    this->head = this->tail = nullptr;
    this->len = 0;
  }

  ~SinglyLinkedList() { clear(); }
};
