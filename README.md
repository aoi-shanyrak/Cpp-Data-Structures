# Data Structures Library (C++)

This library provides templated implementations of modern data structures in C++20 with comprehensive test coverage.

## Data Structures

### Core Collections
- **Vector** — a dynamic array (similar to `std::vector`) with manual memory management and allocator support.
- **LinkedList** — singly and doubly linked lists with custom iterators and node allocator.
- **BitSet** — efficient bit manipulation with comprehensive operations and type conversions.

### Trees
- **Map/Set** — ordered key-value and unique key storage using Red-Black BST.
- **RedBlackBST** — self-balancing binary search tree with O(log n) operations.

### Heaps (Priority Queues)
- **Heap** — binary heap with customizable ordering (min/max).
- **BinaryHeap** — extended heap with handle-based priority updates.
- **BinomialHeap** — logarithmic time merges and efficient operations.
- **FibonacciHeap** — optimal for workloads with frequent `decreasePriority` calls.

---

## Vector

The template class `Vector<T, Allocator>` implements a dynamic array with a classic interface:

- Constructor, destructors, and assignment operators
- `push_back()`, `pop_back()`, `reserve()`, `resize()`, `clear()`
- Random access via `operator[]`
- Bidirectional iterators with range-based for support

---

## LinkedList

The library provides two linked list implementations:

- `SinglyLinkedList<T, A>` — singly linked list with forward iterator.
- `DoublyLinkedList<T, A>` — doubly linked list with bidirectional iterator.

Both implementations use a custom node allocator to reduce syscalls by pre-reserving memory for nodes.

---

## BitSet

The `BitSet` class provides efficient bit manipulation with the following features:

- **Constructors**: default, by size, from `unsigned long long`, from string
- **Bit Operations**: `set()`, `reset()`, `flip()` on all bits or individual positions
- **Status Checks**: `all()`, `any()`, `none()`, `count()`, `empty()`, `size()`
- **Bitwise Operators**: AND, OR, XOR, NOT, left/right shifts
- **Conversions**: to string or `unsigned long long`
- **Type Safety**: proxy reference for safe bit access via `operator[]`
- **Exception Handling**: boundary checks and size validation

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

## Map and Set

The library provides `Map<K, V>` and `Set<K>` implemented using Red-Black binary search trees:

- `Map<K, V>` — ordered key-value pairs with O(log n) lookup and insertion.
- `Set<K>` — ordered unique keys with all BST operations.

Both support iterators, range operations, and standard container semantics.

---

## Red Black BSTree

This class implements a self-balancing binary search tree with the following guarantees:

- O(log n) insertion, deletion, and lookup.
- Automatic balancing to maintain tree height.
- Iterator support for in-order traversal.

---

## Building and Testing

Build all tests:
```bash
make test
```

Run deep tests (longer execution):
```bash
make deeptest
```
