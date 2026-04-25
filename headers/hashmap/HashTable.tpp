#pragma once

#include <cassert>
#include <concepts>
#include <optional>
#include <random>
#include <stdexcept>
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


      HashTableCuckoo(size_t n = default_cap) : data {} {
        size_t cap {Details::next_prime(n)};
        data.resize(cap);
        hasher = HashFamily {cap, N};
      }
      template <std::convertible_to<HashFamily> U_HashFamily>
      explicit HashTableCuckoo(U_HashFamily&& hasher) : data {}, hasher {std::forward<U_HashFamily>(hasher)} {}

      HashTableCuckoo(const HashTableCuckoo& other) = delete;
      HashTableCuckoo& operator=(const HashTableCuckoo&) = delete;

      HashTableCuckoo(HashTableCuckoo&& other) : data {std::move(other.data)}, hasher {std::move(other.hasher)} {
        other.data = {};
        other.hasher = {};
      }
      HashTableCuckoo& operator=(HashTableCuckoo&& other) {
        if (this != other) {
          clear();
          data = std::move(other.data);
          hasher = std::move(other.hasher);
          other.data = {};
          other.hasher = {};
        }
        return *this;
      }
      ~HashTableCuckoo() { clear(); }


      template <std::convertible_to<K> UK, std::convertible_to<T> UT>
      void insert(UK&& key, UT&& value);


      template <std::convertible_to<K> UK>
      T& remove(UK&& key);


      template <typename Self, std::convertible_to<K> UK>
      auto& find(this Self&& self, UK&& key) {
        for (size_t i = 0; i < N; ++i) {
          size_t idx {self.hasher.hash(key, i)};
          auto& slot {self.data[idx]};

          if (slot.has_value() && slot.first == key) return slot.second;
        }
      }


      void clear() noexcept {
        data.clear();
        hasher = {};
      }


     private:
      Container<std::optional<Entry>> data;
      HashFamily hasher;
    };

  }

}
