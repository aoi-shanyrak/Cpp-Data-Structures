#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace aoi {

  namespace Allocator {

    template <typename NodeType, typename A = std::allocator<NodeType>,
              typename ChunkSize = std::integral_constant<size_t, 8>,
              template <typename...> typename Container = std::vector>
    class NodeAllocator {

     private:
      struct FreeNode {
        FreeNode* next;
      };
      static constexpr size_t alignment = std::max(alignof(NodeType), alignof(FreeNode));
      static constexpr size_t node_size =
          (std::max(sizeof(NodeType), sizeof(FreeNode)) + alignment - 1) & ~(alignment - 1);
      static constexpr size_t chunk_size = ChunkSize::value;
      static_assert(chunk_size > 0, "Chunk size must be positive");

      struct alignas(alignment) NodeStorage {
        std::byte data[node_size];
      };
      using ChunkAlloc = typename std::allocator_traits<A>::template rebind_alloc<NodeStorage>;

      ChunkAlloc alloc;
      FreeNode* freelist = nullptr;
      Container<NodeStorage*> chunklist;

      void clear() {
        for (NodeStorage* ptr : chunklist) {
          alloc.deallocate(ptr, chunk_size);
        }
        chunklist.clear();
        freelist = nullptr;
      }


     public:
      NodeAllocator(const A& alloc = A()) : alloc {alloc} {}
      ~NodeAllocator() { clear(); }

      NodeAllocator(const NodeAllocator&) = delete;
      NodeAllocator& operator=(const NodeAllocator&) = delete;

      NodeAllocator(NodeAllocator&& na) noexcept
          : alloc {std::move(na.alloc)}, freelist {na.freelist}, chunklist {std::move(na.chunklist)} {
        na.freelist = nullptr;
      }
      NodeAllocator& operator=(NodeAllocator&& other) noexcept {
        if (this != &other) {
          clear();
          std::swap(freelist, other.freelist);
          alloc = std::move(other.alloc);
          chunklist = std::move(other.chunklist);
          other.freelist = nullptr;
        }
        return *this;
      }

      NodeType* allocate() {
        if (!freelist) {
          NodeStorage* new_chunk = alloc.allocate(chunk_size);
          chunklist.push_back(new_chunk);
          for (size_t i = 0; i < chunk_size; ++i) {
            NodeStorage* storage = &new_chunk[i];
            FreeNode* node = reinterpret_cast<FreeNode*>(storage);
            node->next = freelist;
            freelist = node;
          }
        }
        FreeNode* node = freelist;
        freelist = node->next;
        return reinterpret_cast<NodeType*>(node);
      }

      void deallocate(NodeType* ptr) {
        if (!ptr) return;
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freelist;
        freelist = node;
      }
    };

  }

  namespace NodeType {

    template <typename T>
    class SinglyNode {
     public:
      T data;
      SinglyNode* next;

      SinglyNode(const T& value) : data {value}, next {nullptr} {}
      virtual ~SinglyNode() = default;
    };

    template <typename T>
    class DoublyNode : public SinglyNode<T> {
     public:
      DoublyNode* prev;

      DoublyNode(const T& value) : SinglyNode<T>(value), prev {nullptr} {}
    };

  }

  namespace Base {

    template <typename T>
    class LinkedList_Base {

     protected:
      using Node = NodeType::SinglyNode<T>;

      Node *head, *tail;
      size_t len;

     public:
      LinkedList_Base() : head {nullptr}, tail {nullptr}, len {} {}

      virtual void push_front(T value) = 0;
      virtual void push_back(T value) = 0;

      virtual void pop_front() = 0;
      virtual void pop_back() = 0;

      virtual T& front() = 0;
      virtual T& back() = 0;
      virtual const T& front() const = 0;
      virtual const T& back() const = 0;

      size_t size() { return len; }
      bool empty() { return len == 0; }

      virtual void clear() = 0;
      virtual ~LinkedList_Base() = default;
    };

  }

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

}
