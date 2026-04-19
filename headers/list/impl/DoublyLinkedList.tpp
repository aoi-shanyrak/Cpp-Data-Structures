#pragma once


template <typename T, typename A = std::allocator<T>>
class DoublyLinkedList : public Base::LinkedListBase<T, DoublyLinkedList<T, A>, DoublyNode<T>, A> {
  using BaseList = Base::LinkedListBase<T, DoublyLinkedList<T, A>, DoublyNode<T>, A>;
  using Node = DoublyNode<T>;

  friend BaseList;


 public:
  template <bool IsConst>
  class Iterator : public BaseList::template Iterator<IsConst> {
    using BaseIter = typename BaseList::template Iterator<IsConst>;

   public:
    using iterator_category = std::bidirectional_iterator_tag;

    Iterator() = default;
    explicit Iterator(typename BaseIter::NodePtr ptr) : BaseIter {ptr} {}

    template <bool OtherConst, typename = std::enable_if_t<IsConst && !OtherConst>>
    Iterator(const Iterator<OtherConst>& other) : BaseIter {other} {}

    Iterator& operator--() {
      this->node = Node::prev(*this->node);
      return *this;
    }
    Iterator& operator--(int) {
      Iterator tmp {*this};
      this->node = Node::prev(*this->node);
      return tmp;
    }
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(this->head); }
  iterator end() { return iterator(nullptr); }
  const_iterator begin() const { return const_iterator(this->head); }
  const_iterator end() const { return const_iterator(nullptr); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }


  explicit DoublyLinkedList(const A& alloc = A()) : BaseList {alloc} {}

  void push_front(const T& value) {
    Node* node {this->allocate_node(value)};
    Node::set_next(*node, this->head);
    Node::set_prev(*node, nullptr);

    if (this->head) Node::set_prev(*static_cast<Node*>(this->head), node);
    this->head = node;
    if (!this->tail) this->tail = node;
    ++this->len;
  }

  void push_back(const T& value) override {
    Node* node {this->allocate_node(value)};
    Node::set_next(*node, nullptr);
    Node::set_prev(*node, static_cast<Node*>(this->tail));

    if (this->tail) Node::set_next(*static_cast<Node*>(this->tail), node);
    this->tail = node;
    if (!this->head) this->head = node;
    ++this->len;
  }

  void pop_front() {
    Node* node {static_cast<Node*>(this->head)};
    this->head = Node::next(*node);
    if (this->head)
      Node::set_prev(*static_cast<Node*>(this->head), nullptr);
    else
      this->tail = nullptr;
    this->deallocate_node(node);
    --this->len;
  }

  void pop_back() override {
    Node* node {static_cast<Node*>(this->tail)};
    this->tail = Node::prev(*node);
    if (this->tail)
      Node::set_next(*static_cast<Node*>(this->tail), nullptr);
    else
      this->head = nullptr;
    this->deallocate_node(node);
    --this->len;
  }


  iterator insert(iterator pos, const T& value) {
    Node* target {pos.raw()};
    Node* node {this->allocate_node(value)};

    if (!target) {
      Node::set_next(*node, nullptr);
      Node::set_prev(*node, static_cast<Node*>(this->tail));
      if (this->tail) Node::set_next(*static_cast<Node*>(this->tail), node);
      this->tail = node;
      if (!this->head) this->head = node;

    } else {
      Node* prev_node {Node::prev(*target)};
      Node::set_next(*node, target);
      Node::set_prev(*node, prev_node);
      Node::set_prev(*target, node);
      if (prev_node)
        Node::set_next(*prev_node, node);
      else
        this->head = node;
    }
    ++this->len;
    return iterator(node);
  }

  iterator erase(iterator pos) {
    Node* node {pos.raw()};
    if (!node) return end();

    Node* next_node {Node::next(*node)};
    Node* prev_node {Node::prev(*node)};
    if (prev_node)
      Node::set_next(*prev_node, next_node);
    else
      this->head = next_node;

    if (next_node)
      Node::set_prev(*next_node, prev_node);
    else
      this->tail = prev_node;
    this->deallocate_node(node);
    --this->len;
    return iterator(next_node);
  }
};
