#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
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
      size_t size;

     public:
      template <bool IsConst>
      class Iterator_Base {
       private:
        using NodePtr = std::conditional<IsConst, const Node*, Node*>;
        NodePtr node;

       public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        Iterator_Base() = default;
        explicit Iterator_Base(NodePtr ptr = nullptr) : node(ptr) {}
        Iterator_Base(const Iterator_Base<false>& other)
          requires(IsConst)
            : node(other.node) {}

        reference operator*() const { return node->data; }
        pointer operator->() const { return &node->data; }

        Iterator_Base& operator++() {
          node = node->next;
          return *this;
        }
        Iterator_Base operator++(int) {
          Iterator_Base tmp = *this;
          node = node->next;
          return tmp;
        }

        friend bool operator==(const Iterator_Base& a, const Iterator_Base& b) { return a.node == b.node; }
        friend bool operator!=(const Iterator_Base& a, const Iterator_Base& b) { return a.node != b.node; }
      };
      template <bool>
      friend class Iterator_Base;

      using iterator = Iterator_Base<false>;
      using const_iterator = Iterator_Base<true>;


      iterator begin() { return iterator {head}; }
      iterator end() { return iterator {nullptr}; }
      const_iterator begin() const { return const_iterator {head}; }
      const_iterator end() const { return const_iterator {nullptr}; }

      virtual void push_front(T value) = 0;
      virtual void push_back(T value) = 0;

      virtual T& pop_front() = 0;
      virtual T& pop_back() = 0;
      virtual const T& front() = 0;
      virtual const T& back() = 0;

      size_t size() { return size; }
      bool empty() { return size == 0; }

      virtual void clear() = 0;
      virtual ~LinkedList_Base() = 0;
    };

  }

  template <typename T, typename A = std::allocator<T>>
  class SinglyLinkedList : public Base::LinkedList_Base {
   private:
    using Node = NodeType::SinglyNode<T>;
    using NodeAlloc = Allocator::NodeAllocator<Node, A>;

    NodeAlloc alloc;

   public:
    explicit SinglyLinkedList(const A& alloc = A()) : alloc {alloc} {}

    void push_front(T value) override {
      Node* new_node = alloc.allocate();

      new (new_node) Node {std::move(value)};
      new_node->next = this->head;
      this->head = new_node;

      if (!this->tail) {
        this->tail = new_node;
      }
      ++this->size;
    }
  };

}
