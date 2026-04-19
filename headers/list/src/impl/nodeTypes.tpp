#pragma once

template <typename NodeType, typename T>
concept ListNode = requires(NodeType n, const NodeType cn, T value) {
  { NodeType::value(cn) } -> std::convertible_to<const T&>;
  { NodeType::next(cn) } -> std::convertible_to<NodeType*>;
  { NodeType::set_value(n, std::move(value)) } -> std::same_as<void>;
  { NodeType::set_next(n, std::declval<NodeType*>()) } -> std::same_as<void>;
  { NodeType::construct(std::declval<void*>(), value) } -> std::same_as<void>;
  { NodeType::destroy(std::declval<void*>()) } -> std::same_as<void>;
};


template <typename T>
struct SinglyNode {
  T data;
  SinglyNode* next = nullptr;

  static T& value(SinglyNode& n) { return n.data; }
  static const T& value(const SinglyNode& n) { return n.data; }
  static SinglyNode* next(SinglyNode& n) { return n.next; }
  static const SinglyNode* next(const SinglyNode& n) { return n.next; }
  static void set_value(SinglyNode& n, T&& v) { n.data = std::move(v); }
  static void set_next(SinglyNode& n, SinglyNode* p) { n.next = p; }
  static void construct(void* ptr, T& value) { new (ptr) SinglyNode {value, nullptr}; }
  static void destroy(void* ptr) { static_cast<SinglyNode*>(ptr)->~SinglyNode(); }
};


template <typename T>
struct DoublyNode : SinglyNode<T> {
  DoublyNode* prev = nullptr;

  static DoublyNode* next(DoublyNode& n) { return static_cast<DoublyNode*>(n.next); }
  static const DoublyNode* next(const DoublyNode& n) { return static_cast<const DoublyNode*>(n.next); }
  static DoublyNode* prev(DoublyNode& n) { return static_cast<DoublyNode*>(n.prev); }
  static const DoublyNode* prev(const DoublyNode& n) { return static_cast<const DoublyNode*>(n.prev); }

  static void set_next(DoublyNode& n, DoublyNode* p) { n.next = p; }
  static void set_prev(DoublyNode& n, DoublyNode* p) { n.prev = p; }

  static void construct(void* ptr, T& value) { new (ptr) DoublyNode {value, nullptr, nullptr}; }
};
