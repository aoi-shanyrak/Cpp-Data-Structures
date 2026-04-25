#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../headers/hashmap/HashTable.tpp"

using namespace aoi;

namespace {

  class IntHashFamily {
   public:
    IntHashFamily() = default;
    explicit IntHashFamily(size_t p, size_t) : buckets_ {p} {}

    size_t hash(const int& key, size_t i) const {
      const size_t k = static_cast<size_t>(key < 0 ? -key : key);
      return (k * 2654435761ULL + i * 131ULL) % buckets_;
    }

   private:
    size_t buckets_ {1};
  };

  class RehashTriggerFamily {
   public:
    RehashTriggerFamily() = default;
    explicit RehashTriggerFamily(size_t p, size_t) : buckets_ {p} {}

    size_t hash(const int& key, size_t) const {
      const size_t k = static_cast<size_t>(key < 0 ? -key : key);
      return k % buckets_;
    }

   private:
    size_t buckets_ {1};
  };

  using IntTable = Base::HashTableCuckoo<int, std::string, IntHashFamily>;
  using RehashTable = Base::HashTableCuckoo<int, std::string, RehashTriggerFamily, std::vector, 1>;

  void test_insert_find_update() {
    std::cout << "Testing HashTable insert/find/update... ";

    IntTable table {7};

    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");

    auto* p1 = table.find(1);
    auto* p2 = table.find(2);
    auto* p3 = table.find(3);

    assert(p1 != nullptr && p1->second == "one");
    assert(p2 != nullptr && p2->second == "two");
    assert(p3 != nullptr && p3->second == "three");
    assert(table.find(999) == nullptr);

    table.insert(2, "TWO");
    auto* updated = table.find(2);
    assert(updated != nullptr);
    assert(updated->second == "TWO");

    assert(table.find(2) != nullptr && table.find(2)->second == "TWO");
    assert(table.find(3) != nullptr && table.find(3)->second == "three");

    std::cout << "passed\n";
  }

  void test_move_constructor() {
    std::cout << "Testing HashTable move constructor... ";

    IntTable src {11};
    src.insert(10, "ten");
    src.insert(20, "twenty");

    IntTable moved {std::move(src)};
    assert(moved.find(10) != nullptr && moved.find(10)->second == "ten");
    assert(moved.find(20) != nullptr && moved.find(20)->second == "twenty");

    std::cout << "passed\n";
  }

  void test_rehash_preserves_values() {
    std::cout << "Testing HashTable rehash path... ";

    RehashTable table {3};

    // For N=1 and modulo hash, keys 0 and 5 collide at size 5 and force rehash.
    table.insert(0, "zero");
    table.insert(5, "five");
    table.insert(11, "eleven");

    assert(table.find(0) != nullptr && table.find(0)->second == "zero");
    assert(table.find(5) != nullptr && table.find(5)->second == "five");
    assert(table.find(11) != nullptr && table.find(11)->second == "eleven");

    std::cout << "passed\n";
  }

  void test_string_hash_map_alias() {
    std::cout << "Testing StringHashMap alias (std::string)... ";

    StringHashMap<int> map {11};

    map.insert("C++", 20);
    map.insert("Rust", 2010);
    map.insert("Python", 1991);

    auto* p = map.find("C++");
    assert(p != nullptr && p->second == 20);
    assert(map.find("Java") == nullptr);

    std::cout << "passed\n";
  }

  void test_remove_functionality() {
    std::cout << "Testing HashTable remove... ";

    IntTable table {13};
    table.insert(100, "hundred");
    table.insert(200, "two-hundred");

    assert(table.find(100) != nullptr);

    table.remove(100);
    assert(table.find(100) == nullptr);
    assert(table.find(200) != nullptr);

    table.remove(999);

    std::cout << "passed\n";
  }

  void test_large_scale_rehash() {
    std::cout << "Testing Large scale insertions (forces multiple rehashes)... ";

    StringHashMap<size_t> map {5};
    const size_t count = 500;

    for (size_t i = 0; i < count; ++i) {
      map.insert("key_" + std::to_string(i), i);
    }

    for (size_t i = 0; i < count; ++i) {
      auto* p = map.find("key_" + std::to_string(i));
      assert(p != nullptr && p->second == i);
    }

    std::cout << "passed\n";
  }

  void test_clear_functionality() {
    std::cout << "Testing HashTable clear... ";

    IntTable table {11};
    table.insert(1, "one");
    table.insert(2, "two");

    table.clear();

    assert(table.find(1) == nullptr);
    assert(table.find(2) == nullptr);

    table.insert(3, "three");
    assert(table.find(3) != nullptr);

    std::cout << "passed\n";
  }

}  // namespace

int main() {
  std::cout << "=== HashTable Test Suite ===\n\n";

  try {
    test_insert_find_update();
    test_move_constructor();
    test_rehash_preserves_values();
    test_string_hash_map_alias();
    test_remove_functionality();
    test_clear_functionality();
    test_large_scale_rehash();

    std::cout << "\n=== All HashTable tests passed! ===\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\nTest failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\nTest failed with unknown exception\n";
    return 1;
  }
}
