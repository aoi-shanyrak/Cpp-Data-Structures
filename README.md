# Data Structures Library (C++)

This library provides templated implementations of three data structures in C++17:

- **Vector** — a dynamic array (similar to `std::vector`) with manual memory management with allocator.
- **Heap** — a classic binary heap (priority queue) with support for custom comparators.
- **HeapWithInc** — an extended version of the heap that allows increasing the priority of an arbitrary element (decrease-key) using an index map.
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

The `Heap<T, Priority = int, Compare = std::greater<Priority>, Allocator = ...>` class implements a binary heap (priority queue).

- By default, it works as a **max-heap** (higher priority comes first).
- You can specify a custom comparator (e.g., `std::less<int>` for a min-heap).
- Operations: `push(priority, value)`, `pop()`, `peek()`.

Example:

```cpp
Heap<std::string> h;
h.push(10, "high");
h.push(5,  "medium");
std::cout << h.peek(); // "high"
```

---

## HeapWithInc

The `HeapWithInc<T, Priority = int, Compare = std::greater<Priority>, Allocator = ...>` class extends the standard heap by allowing **priority increases** for elements already in the heap (similar to decrease-key in Dijkstra's algorithm). This is achieved using an additional `indexMap` vector that stores the position of each element in the heap.

- The constructor requires specifying `maxSize` — the maximum number of unique values (the size of `indexMap`).
- Methods:
  - `containsValue(value)` – checks if an element is present in the heap.
  - `increasePriorityByValue(value, newPriority)` – increases the priority of an element by its value.
  - `increasePriority(index, newPriority)` – by its index in the heap.
  - `peekWithPriority()` and `popWithPriority()` return pairs.
- All heap-modifying operations (`push`, `pop`, `increasePriority`) automatically update `indexMap`.

Example (simulating Dijkstra on a min-heap):

```cpp
HeapWithInc<int, int, std::less<int>> pq(6); // 6 vertices, min-heap
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

```bash
make
```
