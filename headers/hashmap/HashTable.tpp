#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace aoi {


  namespace Details {

    constexpr bool is_prime(unsigned long long int x) {
      if (x < 2) return false;
      if (x % 2 == 0) return x == 2;
      for (unsigned long long int d = 3; d * d <= x; d += 2)
        if (x % d == 0) return false;
      return true;
    }

    constexpr unsigned long long int next_prime(unsigned long long int x) {
      unsigned long long int n {x + 1};
      if (n % 2 == 0) ++n;
      while (!is_prime(n)) n += 2;
      return n;
    }

  }


  namespace Hashing {

    template <typename String>
    class StringHashFamily {
     public:
      StringHashFamily() = delete;
      explicit StringHashFamily(size_t p, size_t k) {
        if (!Details::is_prime(p) || p < 5 || k <= 0)
          throw std::runtime_error("aoi::Hashing::StringHashFamily::constructor: P must be prime");

        buckets = p;

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long int> dist(2, buckets - 1);

        seeds.reserve(k);
        for (size_t i = 0; i < k; ++i) seeds.push_back(dist(gen));
      }
      StringHashFamily(const StringHashFamily&) = default;
      StringHashFamily& operator=(const StringHashFamily&) = default;
      StringHashFamily(StringHashFamily&&) = default;
      StringHashFamily& operator=(StringHashFamily&&) = default;
      ~StringHashFamily() = default;

      unsigned long long int hash(const String& key, size_t i) const {
        unsigned long long int h {0xA01 * i}, a {seeds[i % seeds.size()]};
        for (const auto& x : key) h = (h * a) + static_cast<unsigned char>(x);
        return h % buckets;
      }

     private:
      size_t buckets;
      std::vector<unsigned long long int> seeds;
    };

  }


  namespace Base {

    template <typename K, typename T, typename HashFamily, template <typename...> typename Container = std::vector,
              size_t N = 2>
    class HashTableCuckoo {
      using Entry = std::pair<K, T>;
      constexpr static size_t default_cap = 101;

     public:
      using Handle = Entry*;


      HashTableCuckoo(size_t n = default_cap) : data {Details::next_prime(n)}, hasher {data.size(), N} {}
      template <std::convertible_to<HashFamily> U_HashFamily>
      explicit HashTableCuckoo(U_HashFamily&& hasher) : data {}, hasher {std::forward<U_HashFamily>(hasher)} {}

      HashTableCuckoo(const HashTableCuckoo& other) = delete;
      HashTableCuckoo& operator=(const HashTableCuckoo&) = delete;

      HashTableCuckoo(HashTableCuckoo&& other) : data {std::move(other.data)}, hasher {std::move(other.hasher)} {
        other.data = {};
      }
      HashTableCuckoo& operator=(HashTableCuckoo&& other) {
        if (this != other) {
          clear();
          data = std::move(other.data);
          hasher = std::move(other.hasher);
          other.data = {};
        }
        return *this;
      }
      ~HashTableCuckoo() { clear(); }

      void clear() noexcept { data.clear(); }


      template <std::convertible_to<K> UK, std::convertible_to<T> UT>
      void insert(UK&& key, UT&& value);


      template <std::convertible_to<K> UK>
      void remove(UK&& key) {
        auto* slot {find_slot(std::forward<UK>(key))};
        if (slot == nullptr) return;
        slot->reset();
      }


      template <std::convertible_to<K> UK>
      Handle find(UK&& key) {
        auto* slot {find_slot(std::forward<UK>(key))};
        return (slot) ? &(**slot) : nullptr;
      }


     private:
      Container<std::optional<Entry>> data;
      HashFamily hasher;


      template <std::convertible_to<K> UK>
      std::optional<Entry>* find_slot(UK&& key) {
        for (size_t i = 0; i < N; ++i) {
          size_t idx {hasher.hash(key, i)};
          auto& slot {data[idx]};
          if (slot.has_value() && slot->first == key) return &slot;
        }
        return nullptr;
      }

      constexpr unsigned long long int max_loop() { return std::log2(data.size()); }

      void rehash() {
        size_t new_size {Details::next_prime(data.size() * 2)};
        auto old_data {std::move(data)};

        hasher = HashFamily {new_size, N};

        data.resize(new_size);
        for (auto& slot : old_data)
          if (slot.has_value()) insert(std::move(slot->first), std::move(slot->second));
      }
    };


    template <typename K, typename T, typename HashFamily, template <typename...> typename Container, size_t N>
    template <std::convertible_to<K> UK, std::convertible_to<T> UT>
    void HashTableCuckoo<K, T, HashFamily, Container, N>::insert(UK&& key, UT&& value) {
      if (auto pair = find(std::forward<UK>(key))) {
        pair->second = std::forward<UT>(value);
        return;
      }

      Entry current {std::forward<UK>(key), std::forward<UT>(value)};
      size_t limit {max_loop()};

      for (size_t i = 0; i < limit; ++i) {
        for (size_t j = 0; j < N; ++j) {
          size_t idx {hasher.hash(current.first, j)};
          auto& slot {data[idx]};
          if (!slot.has_value()) {
            slot = std::move(current);
            return;
          }
          std::swap(*slot, current);
        }
      }
      rehash();

      insert(std::move(current.first), std::move(current.second));
    }

  }


  template <typename T>
  using StringHashMap = Base::HashTableCuckoo<std::string, T, Hashing::StringHashFamily<std::string>>;

}
