# Data Structures Library (C++)

This library provides templated implementations of three data structures in C++17:

- Simple:
  - **Vector** — a dynamic array (similar to `std::vector`) with manual memory management with allocator.
  - **Heap** — binary heap with customizable ordering.
  - **LinkedList** — singly and doubly linked lists with custom iterators and node allocator.
- Heaps:
  - **BinaryHeap** -- a extended version of `Heap` with support for decreasing\deleting by handles
  - **BinomialHeap** -- a classic binomial heap (all operations for O(logN))
  - **FibonacciHeap** -- a masterpiece of all heaps

---

## Vector

The template class `Vector<T, Allocator>` implements a dynamic array with a classic interface:

---

## Heap

The `Heap<T, Priority = int, Compare = std::less<Priority>, Allocator = ...>` class implements a binary heap (priority queue).

- Default mode: **min-heap**.
- Custom comparator enables max-heap behavior.
- Core operations: `push`, `pop`, `peek`, `merge`.

---

## Quick Guide: Advanced Heaps

### BinaryHeap

- Use when you need fast handle-based updates.
- Main operations: `push`, `peek`, `pop`, `decreasePriority`, `deleteKey`.

### BinomialHeap

- Good when frequent heap merges are needed.
- Main operations: `push`, `peek`, `pop`, `merge`.

### FibonacciHeap

- Best choice for many `decreasePriority` calls.
- Main operations: `push`, `peek`, `pop`, `decreasePriority`, `deleteKey`, `merge`.

Tip: use `std::greater<int>` as comparator to switch to max-heap behavior.

---

## LinkedList

The library provides two linked list implementations:

- `SinglyLinkedList<T, A>` — singly linked list with forward iterator.
- `DoublyLinkedList<T, A>` — doubly linked list with bidirectional iterator.

This linked list also has his own allocator based on import allocator. So it reduce amount of syscalls by reserving memory for nodes.

---

## Building and Testing

All tests are made by ai.

```bash
make
```
