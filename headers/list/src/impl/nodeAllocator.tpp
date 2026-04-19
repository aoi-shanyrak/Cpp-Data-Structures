#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

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
