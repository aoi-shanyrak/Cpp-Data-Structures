#pragma once

#include <bitset>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <numbers>
#include <utility>

namespace aoi {

  template <typename T, size_t K, size_t N, typename HashFamily, typename BitStorage = std::bitset<N>>
  class BloomFilter {
   public:
    static_assert(K > 0 && N > 0, "K and N must be positive");

    BloomFilter() : bits {} {}


    template <std::convertible_to<T> UT>
    void insert(UT&& item) {
      setBits(item, std::make_index_sequence<K> {});
    }

    template <std::convertible_to<T> UT>
    bool contains(UT&& item) {
      return checkBits(item, std::make_index_sequence<K> {});
    }


   private:
    BitStorage bits;
    HashFamily hasher;


    template <std::convertible_to<T> UT, size_t... I>
    void setBits(UT&& item, std::index_sequence<I...>) {
      (bits.set(hasher(item, I)), ...);
    }

    template <std::convertible_to<T> UT, size_t... I>
    bool checkBits(UT&& item, std::index_sequence<I...>) {
      return (bits.test(hasher(item, I)) && ...);
    }
  };


  namespace Detail {

    inline size_t calcK(double E) {
      return static_cast<size_t>(std::floor(-std::log2(E)) + 1);
    }

    inline size_t calcN(size_t K, size_t S) {
      return static_cast<size_t>(std::ceil(K * S * std::numbers::log2e));
    }

  }

}
