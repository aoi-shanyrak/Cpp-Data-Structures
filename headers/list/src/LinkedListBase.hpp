#pragma once

#include <cstddef>


#include "impl/nodeTypes.tpp"
namespace aoi {

#include "impl/nodeAllocator.tpp"


  namespace Base {

    template <typename T, typename Derived, typename NodeType, typename A = std::allocator<T>>
    class LinkedListBase {

     protected:
      using Node = NodeT;
      using NodeAlloc = Allocator::NodeAllocator<Node, A>;
      NodeAlloc alloc;
      Node* head;
      Node* tail;
      size_t len;

     public:
      explicit LinkedListBase(const A& allocator = A()) : alloc {allocator}, head {nullptr}, tail {nullptr}, len {} {}

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
      virtual ~LinkedListBase() = default;
    };

  }

}
