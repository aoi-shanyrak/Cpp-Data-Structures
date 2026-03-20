#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace aoi {

  template <typename NodeType, typename A = std::allocator<NodeType>,
            template <typename...> typename Container = std::vector>
  class NodeAllocator {
   private:
    static constexpr size_t alignment = std::max(alignof(NodeType), alignof(FreeNode));
    static constexpr size_t node_size =
        (std::max(sizeof(NodeType), sizeof(FreeNode)) + alignment - 1) & ~(alignment - 1);

    struct NodeStorage {
      alignas(alignment) std::byte data[node_size];
    };
    using BlockAlloc = typename std::allocator_traits<A>::template rebind_alloc<NodeStorage>;

    BlockAlloc alloc;

    Container<NodeStorage*> chunklist;
    const size_t chunk_size = 16;

    struct FreeNode {
      FreeNode* next;
    };
    FreeNode* freelist = nullptr;


    void clear() {
      for (NodeStorage* ptr : chunklist) {
        alloc.deallocate(ptr, chunk_size * node_size);
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
        : freelist {na.freelist}, alloc {std::move(na.alloc)}, chunklist {std::move(na.chunklist)} {
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

    NodeType* allocate();

    void deallocate(NodeType* ptr) {
      if (!ptr) return;
      FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
      node->next = freelist;
      freelist = node;
    }
  };

  template <typename T, typename A = std::allocator<T>>
  class LinkedList_Base {};

}
