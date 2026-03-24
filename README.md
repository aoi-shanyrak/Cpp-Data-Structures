# Data Structures Library (C++)

This library provides templated implementations of three data structures in C++17:

- **Vector** — a dynamic array (similar to `std::vector`) with manual memory management with allocator.
- **Heap** — a classic binary heap (priority queue) with support for custom comparators.
- **HeapDecreasing** — an extended version of the heap that allows decreasing the priority of an arbitrary element (decrease-key) using an index map.
- **LinkedList** — singly and doubly linked lists with custom iterators and node allocator.

---

## Vector

The template class `Vector<T, Allocator>` implements a dynamic array with a classic interface:

```cpp
Vector<int> v(5, 42);      // 5 elements with value 42
v.push_back(100);           // add an element
for (int x : v) { /* ... */ }
```

---

## Heap

The `Heap<T, Priority = int, Compare = std::less<Priority>, Allocator = ...>` class implements a binary heap (priority queue).

- By default, it works as a **min-heap** (lower priority comes first).
- You can specify a custom comparator (e.g., `std::greater<int>` for a max-heap).
- Operations: `push(priority, value)`, `pop()`, `peek()`, `merge(Heap)`.

Example:

```cpp
Heap<std::string, int, std::greater<int>> h;
h.push(10, "high");
h.push(5,  "medium");
std::cout << h.peek(); // "high"
```

---

## HeapDecreasing

The `HeapDecreasing<T, Priority = int, Compare = std::greater<Priority>, Allocator = ...>` class extends the standard heap by allowing **priority decreases** for elements already in the heap (similar to decrease-key in Dijkstra's algorithm). This is achieved using an additional `indexMap` vector that stores the position of each element in the heap.

- Methods:
  - `containsValue(value)` – checks if an element is present in the heap.
  - `decreasePriorityByValue(value, newPriority)` – increases the priority of an element by its value.
  - `decreasePriority(index, newPriority)` – by its index in the heap.
  - `peekWithPriority()` return pair.
  - `merge(Heap)` merges two heaps into one.
- All heap-modifying operations (`push`, `pop`, `decreasePriority`) automatically update `indexMap`.

Example (simulating Dijkstra on a min-heap):

```cpp
HeapDecreasing<int> pq(6); // 6 vertices, min-heap
pq.push(0, 0);   // vertex 0, distance 0
pq.push(1000, 1); // vertex 1, distance "infinity"
pq.increasePriorityByValue(1, 4); // decrease distance to 4 (in min-heap, increasing the number = lowering priority)
```

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
