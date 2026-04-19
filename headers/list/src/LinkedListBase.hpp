#pragma once

#include <cstddef>

namespace aoi {

#include "impl/nodeTypes.tpp"

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

}
