#pragma once


template <typename T, typename A = std::allocator<T>>
class SinglyLinkedList : public Base::LinkedListBase<T, SinglyLinkedList<T, A>, SinglyNode<T>, A> {
  using BaseList = Base::LinkedListBase<T, SinglyLinkedList<T, A>, SinglyNode<T>, A>;
  using Node = SinglyNode<T>;

  friend BaseList;


 public:
  explicit SinglyLinkedList(const A& alloc = A()) : BaseList {alloc} {}


  void push_back(const T& value) override {
    Node* node {this->allocate_node(value)};
    Node::set_next(*node, nullptr);
    if (!this->head)
      this->head = this->tail = node;
    else {
      Node::set_next(*this->tail, node);
      this->tail = node;
    }
    ++this->len;
  }

  void pop_back() override {
    if (this->len == 1) {
      this->pop_front();
      return;
    }
    Node* prev {this->head};
    while (Node::next(*prev) != this->tail) {
      prev = Node::next(*prev);
    }
    this->deallocate_node(this->tail);
    this->tail = prev;
    Node::set_next(*prev, nullptr);
    --this->len;
  }


  typename BaseList::iterator insert_after(typename BaseList::iterator pos, const T& value) {
    Node* target {pos.raw()};
    if (!target) return this->end();

    Node* node {this->allocate_node(value)};
    Node::set_next(*node, Node::next(*target));
    Node::set_next(*target, node);

    if (!Node::next(*node)) this->tail = node;
    ++this->len;

    return typename BaseList::iterator(node);
  }

  typename BaseList::iterator erase_after(typename BaseList::iterator pos) {
    Node* target {pos.raw()};
    if (!target || !Node::next(*target)) return this->end();

    Node* to_del {Node::next(*target)};
    Node::set_next(*target, Node::next(*to_del));

    if (!Node::next(*target)) this->tail = target;
    this->deallocate_node(to_del);
    --this->len;
    return typename BaseList::iterator(Node::next(*target));
  }
};
