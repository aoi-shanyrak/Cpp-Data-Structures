#pragma once

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
