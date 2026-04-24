#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace aoi {

  class BitSet {
    using bitStorage_t = uint64_t;
    static constexpr size_t BITS_PER_WORD = 64;

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


    BitSet() noexcept : bit_count {0} {}

    explicit BitSet(size_t n) : bit_count {n} { data.resize(words_in_bits(n), 0); }

    explicit BitSet(unsigned long long val)
        : bit_count {static_cast<size_t>(std::bit_width(val))}, data(words_in_bits(bit_count), 0) {
      if (!data.empty()) data[0] = val;
      sanitize_tail();
    }

    explicit BitSet(const std::string& str, size_t pos = 0, size_t n = std::string::npos, char zero = '0',
                    char one = '1')
        : BitSet {} {
      if (pos >= str.size()) throw std::out_of_range("BitSet: Position out of range");

      size_t end = std::min(pos + n, str.size());
      bit_count = end - pos;
      data.resize(words_in_bits(bit_count), 0);

      for (size_t i = 0; i < bit_count; ++i) {
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


    size_t size() const noexcept { return bit_count; }
    bool empty() const noexcept { return bit_count == 0; }

    bool operator[](size_t pos) const noexcept { return (data[pos / BITS_PER_WORD] >> (pos % BITS_PER_WORD)) & 1ULL; }

    reference operator[](size_t pos) noexcept { return reference {this, pos}; }


    bool test(size_t pos) const {
      if (pos >= bit_count) throw std::out_of_range("BitSet: Position out of range");
      return (*this)[pos];
    }

    BitSet& set() noexcept {
      std::fill(data.begin(), data.end(), ~0ULL);
      sanitize_tail();
      return *this;
    }

    BitSet& reset() noexcept {
      std::fill(data.begin(), data.end(), 0ULL);
      return *this;
    }

    BitSet& flip() noexcept {
      for (auto& word : data) word = ~word;
      sanitize_tail();
      return *this;
    }

    BitSet& set(size_t pos, bool value = true) {
      if (pos >= bit_count) throw std::out_of_range("BitSet: Position out of range");
      if (value)
        data[pos / BITS_PER_WORD] |= (1ULL << (pos % BITS_PER_WORD));
      else
        data[pos / BITS_PER_WORD] &= ~(1ULL << (pos % BITS_PER_WORD));
      return *this;
    }

    BitSet& reset(size_t pos) { return set(pos, false); }

    BitSet& flip(size_t pos) {
      if (pos >= bit_count) throw std::out_of_range("BitSet: Position out of range");
      data[pos / BITS_PER_WORD] ^= (1ULL << (pos % BITS_PER_WORD));
      return *this;
    }


    bool all() const noexcept {
      if (bit_count == 0) return true;
      for (size_t i = 0; i < data.size() - 1; ++i) {
        if (data[i] != ~0ULL) return false;
      }
      size_t tail_bits = bit_count % BITS_PER_WORD;
      bitStorage_t mask = (tail_bits == 0) ? ~0ULL : (1ULL << tail_bits) - 1;
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
      result.reserve(bit_count);
      for (size_t i = 0; i < bit_count; ++i) {
        result.push_back((*this)[i] ? one : zero);
      }
      return result;
    }

    unsigned long long to_ullong() const {
      if (bit_count > BITS_PER_WORD) throw std::overflow_error("BitSet: Too large for ullong");
      return data.empty() ? 0ULL : data[0];
    }


    BitSet& operator&=(const BitSet& other) {
      check_size_match(other);
      for (size_t i = 0; i < data.size(); ++i) data[i] &= other.data[i];
      return *this;
    }

    BitSet& operator|=(const BitSet& other) {
      check_size_match(other);
      for (size_t i = 0; i < data.size(); ++i) data[i] |= other.data[i];
      return *this;
    }

    BitSet& operator^=(const BitSet& other) {
      check_size_match(other);
      for (size_t i = 0; i < data.size(); ++i) data[i] ^= other.data[i];
      return *this;
    }

    BitSet& operator<<=(size_t shift);
    BitSet& operator>>=(size_t shift);

    BitSet operator~() const {
      BitSet result {*this};
      result.flip();
      return result;
    }

    bool operator==(const BitSet& other) const {
      check_size_match(other);
      return data == other.data;
    }

    bool operator!=(const BitSet& other) const { return !(*this == other); }


    void swap(BitSet& other) noexcept {
      std::swap(data, other.data);
      std::swap(bit_count, other.bit_count);
    }


   private:
    size_t bit_count;
    std::vector<bitStorage_t> data;


    void sanitize_tail() noexcept {
      if (bit_count == 0) return;
      size_t tail_bits = bit_count % BITS_PER_WORD;
      if (tail_bits > 0) {
        bitStorage_t mask = (1ULL << tail_bits) - 1;
        data.back() &= mask;
      }
    }

    void check_size_match(const BitSet& other) const {
      if (bit_count != other.bit_count) throw std::invalid_argument("BitSet: Size mismatch");
    }

    static size_t words_in_bits(size_t bits) noexcept { return (bits + BITS_PER_WORD - 1) / BITS_PER_WORD; }
  };


  inline BitSet& BitSet::operator<<=(size_t shift) {
    if (bit_count == 0 || shift == 0) return *this;
    if (shift >= bit_count) {
      reset();
      return *this;
    }

    size_t word_shift = shift / BITS_PER_WORD;
    size_t bit_shift = shift % BITS_PER_WORD;

    if (bit_shift == 0) {
      for (size_t i = data.size(); i-- > word_shift;) data[i] = data[i - word_shift];
    } else {
      for (size_t i = data.size(); i-- > word_shift;) {
        bitStorage_t high = data[i - word_shift];
        bitStorage_t low = (i > word_shift) ? data[i - word_shift - 1] : 0;
        data[i] = (high << bit_shift) | (low >> (BITS_PER_WORD - bit_shift));
      }
    }
    std::fill(data.begin(), data.begin() + word_shift, 0);
    sanitize_tail();
    return *this;
  }

  inline BitSet& BitSet::operator>>=(size_t shift) {
    if (bit_count == 0 || shift == 0) return *this;
    if (shift >= bit_count) {
      reset();
      return *this;
    }

    size_t word_shift = shift / BITS_PER_WORD;
    size_t bit_shift = shift % BITS_PER_WORD;

    if (bit_shift == 0) {
      for (size_t i = 0; i + word_shift < data.size(); ++i) data[i] = data[i + word_shift];
    } else {
      for (size_t i = 0; i + word_shift < data.size(); ++i) {
        bitStorage_t low = data[i + word_shift] >> bit_shift;
        bitStorage_t high =
            (i + word_shift + 1 < data.size()) ? data[i + word_shift + 1] << (BITS_PER_WORD - bit_shift) : 0;
        data[i] = low | high;
      }
    }
    std::fill(data.end() - word_shift, data.end(), 0);
    sanitize_tail();
    return *this;
  }


  inline BitSet operator&(const BitSet& a, const BitSet& b) {
    return BitSet(a) &= b;
  }
  inline BitSet operator|(const BitSet& a, const BitSet& b) {
    return BitSet(a) |= b;
  }
  inline BitSet operator^(const BitSet& a, const BitSet& b) {
    return BitSet(a) ^= b;
  }
  inline BitSet operator<<(const BitSet& a, size_t shift) {
    return BitSet(a) <<= shift;
  }
  inline BitSet operator>>(const BitSet& a, size_t shift) {
    return BitSet(a) >>= shift;
  }
  inline std::ostream& operator<<(std::ostream& os, const BitSet& bs) {
    return os << bs.to_string();
  }

}
