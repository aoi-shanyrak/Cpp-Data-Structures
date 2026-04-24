#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string>

namespace aoi {

  template <size_t N>
  class BitSet {
    using bitStorage_t = uint64_t;
    static constexpr size_t BITS_PER_WORD = 64;
    static constexpr size_t WORD_COUNT = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;


   public:
    class reference {
      friend class BitSet;
      BitSet* bs;
      size_t pos;

     public:
      reference(BitSet* bs, size_t pos) noexcept : bs {bs}, pos {pos} {}

      operator bool() const { return bs->test(pos); }

      reference& operator=(bool value) {
        bs->set(pos, value);
        return *this;
      }
      reference& operator=(const reference& other) { return *this = bool(other); }

      reference& flip() {
        bs->flip(pos);
        return *this;
      }

      bool operator~() const { return !bool(*this); }
    };


    BitSet() noexcept { data.fill(0); }

    explicit BitSet(unsigned long long val) noexcept {
      data.fill(0);
      if constexpr (N > 0) {
        data[0] = val;
        sanitize_tail();
      }
    }

    explicit BitSet(const std::string& str, size_t pos = 0, size_t n = std::string::npos, char zero = '0',
                    char one = '1')
        : BitSet {} {
      if (pos >= str.size()) throw std::out_of_range("BitSet: Position out of range");

      size_t rlen = std::min({n, str.size() - pos, N});
      for (size_t i = 0; i < rlen; ++i) {
        char c = str[pos + i];
        if (c == one)
          set(i, true);
        else if (c != zero)
          throw std::invalid_argument("BitSet: Invalid character");
      }
    }

    BitSet(const BitSet& other) = default;
    BitSet(BitSet&& other) noexcept = default;
    BitSet& operator=(const BitSet& other) = default;
    BitSet& operator=(BitSet&& other) noexcept = default;


    constexpr size_t size() const noexcept { return N; }
    constexpr bool empty() const noexcept { return N == 0; }

    bool operator[](size_t pos) const noexcept { return (data[pos / BITS_PER_WORD] >> (pos % BITS_PER_WORD)) & 1ULL; }

    reference operator[](size_t pos) noexcept { return reference {this, pos}; }


    bool test(size_t pos) const {
      if (pos >= N) throw std::out_of_range("BitSet: Position out of range");
      return (*this)[pos];
    }

    BitSet& set() noexcept {
      data.fill(~0ULL);
      sanitize_tail();
      return *this;
    }

    BitSet& reset() noexcept {
      data.fill(0);
      return *this;
    }

    BitSet& flip() noexcept {
      for (auto& word : data) word = ~word;
      sanitize_tail();
      return *this;
    }

    BitSet& set(size_t pos, bool value = true) {
      if (pos >= N) throw std::out_of_range("BitSet: Position out of range");
      if (value)
        data[pos / BITS_PER_WORD] |= (1ULL << (pos % BITS_PER_WORD));
      else
        data[pos / BITS_PER_WORD] &= ~(1ULL << (pos % BITS_PER_WORD));
      return *this;
    }

    BitSet& reset(size_t pos) { return set(pos, false); }

    BitSet& flip(size_t pos) {
      if (pos >= N) throw std::out_of_range("BitSet: Position out of range");
      data[pos / BITS_PER_WORD] ^= (1ULL << (pos % BITS_PER_WORD));
      return *this;
    }


    bool all() const noexcept {
      if constexpr (N == 0) return true;
      for (size_t i = 0; i < data.size() - 1; ++i) {
        if (data[i] != ~0ULL) return false;
      }
      bitStorage_t mask {get_tail_mask()};
      return (data.back() & mask) == mask;
    }

    bool any() const noexcept {
      for (const auto& word : data)
        if (word != 0) return true;
      return false;
    }

    bool none() const noexcept { return !any(); }

    size_t count() const noexcept {
      size_t total = 0;
      for (const auto& word : data) total += std::popcount(word);
      return total;
    }


    std::string to_string(char zero = '0', char one = '1') const {
      std::string result;
      result.reserve(N);
      for (size_t i = 0; i < N; ++i) {
        result.push_back((*this)[i] ? one : zero);
      }
      return result;
    }

    unsigned long long to_ullong() const {
      if constexpr (N == 0) return 0;
      if constexpr (N > BITS_PER_WORD)
        for (size_t i = 1; i < WORD_COUNT; ++i)
          if (data[i] > 0) throw std::overflow_error("BitSet: Too large for ullong");
      return data.empty() ? 0ULL : data[0];
    }


    BitSet& operator&=(const BitSet& other) {
      for (size_t i = 0; i < WORD_COUNT; ++i) data[i] &= other.data[i];
      return *this;
    }

    BitSet& operator|=(const BitSet& other) {
      for (size_t i = 0; i < WORD_COUNT; ++i) data[i] |= other.data[i];
      return *this;
    }

    BitSet& operator^=(const BitSet& other) {
      for (size_t i = 0; i < WORD_COUNT; ++i) data[i] ^= other.data[i];
      return *this;
    }

    BitSet& operator<<=(size_t shift);
    BitSet& operator>>=(size_t shift);

    BitSet operator~() const {
      BitSet result {*this};
      result.flip();
      return result;
    }

    bool operator==(const BitSet& other) const { return data == other.data; }
    bool operator!=(const BitSet& other) const { return !(*this == other); }


   private:
    std::array<bitStorage_t, WORD_COUNT> data;


    static constexpr bitStorage_t get_tail_mask() {
      constexpr size_t tail_bits {N % BITS_PER_WORD};
      return (tail_bits == 0) ? ~0ULL : (1ULL << tail_bits) - 1;
    }
    void sanitize_tail() noexcept {
      if constexpr (N > 0) data.back() &= get_tail_mask();
    }
  };


  template <size_t N>
  auto BitSet<N>::operator<<=(size_t shift) -> BitSet& {
    if (shift == 0) return *this;
    if (shift >= N) {
      reset();
      return *this;
    }

    size_t word_shift = shift / BITS_PER_WORD;
    size_t bit_shift = shift % BITS_PER_WORD;

    if (bit_shift == 0) {
      for (size_t i = WORD_COUNT; i-- > word_shift;) data[i] = data[i - word_shift];
    } else {
      for (size_t i = WORD_COUNT; i-- > word_shift;) {
        bitStorage_t high = data[i - word_shift];
        bitStorage_t low = (i > word_shift) ? data[i - word_shift - 1] : 0;
        data[i] = (high << bit_shift) | (low >> (BITS_PER_WORD - bit_shift));
      }
    }
    std::fill(data.begin(), data.begin() + word_shift, 0);
    sanitize_tail();
    return *this;
  }

  template <size_t N>
  auto BitSet<N>::operator>>=(size_t shift) -> BitSet& {
    if (shift == 0) return *this;
    if (shift >= N) {
      reset();
      return *this;
    }

    size_t word_shift = shift / BITS_PER_WORD;
    size_t bit_shift = shift % BITS_PER_WORD;

    if (bit_shift == 0) {
      for (size_t i = 0; i + word_shift < WORD_COUNT; ++i) data[i] = data[i + word_shift];
    } else {
      for (size_t i = 0; i + word_shift < WORD_COUNT; ++i) {
        bitStorage_t low = data[i + word_shift] >> bit_shift;
        bitStorage_t high =
            (i + word_shift + 1 < WORD_COUNT) ? data[i + word_shift + 1] << (BITS_PER_WORD - bit_shift) : 0;
        data[i] = low | high;
      }
    }
    std::fill(data.end() - word_shift, data.end(), 0);
    sanitize_tail();
    return *this;
  }


  template <size_t N>
  inline BitSet<N> operator&(const BitSet<N>& a, const BitSet<N>& b) {
    return BitSet<N>(a) &= b;
  }
  template <size_t N>
  inline BitSet<N> operator|(const BitSet<N>& a, const BitSet<N>& b) {
    return BitSet<N>(a) |= b;
  }
  template <size_t N>
  inline BitSet<N> operator^(const BitSet<N>& a, const BitSet<N>& b) {
    return BitSet<N>(a) ^= b;
  }
  template <size_t N>
  inline BitSet<N> operator<<(const BitSet<N>& a, size_t shift) {
    return BitSet<N>(a) <<= shift;
  }
  template <size_t N>
  inline BitSet<N> operator>>(const BitSet<N>& a, size_t shift) {
    return BitSet<N>(a) >>= shift;
  }
  template <size_t N>
  inline std::ostream& operator<<(std::ostream& os, const BitSet<N>& bs) {
    return os << bs.to_string();
  }
}
