#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace aoi {

  class BitSet {
    using bitStorage_t = uint8_t;

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
    explicit BitSet(size_t n) : bit_count {n}, data {} {
      size_t byte_count {bytes_in_bits(n)};
      data.resize(byte_count, 0);
    }
    explicit BitSet(unsigned long long val) : BitSet {} {
      size_t length_inbits {get_length_inBits_ULL(val)};
      bit_count = length_inbits;
      size_t byte_count {bytes_in_bits(length_inbits)};
      data.resize(byte_count, 0);

      for (size_t i = 0; i < sizeof(val) && i < byte_count; ++i) {
        data[i] = static_cast<bitStorage_t>(val & 0xFF);
        val >>= 8;
      }
      sanitize_tail();
    }
    explicit BitSet(const std::string& str, size_t pos = 0, size_t n = std::string::npos, char zero = '0',
                    char one = '1')
        : BitSet {} {
      if (pos >= str.size()) throw std::out_of_range("BitSet::BitSet: Position is out of range");

      size_t end {std::min(pos + n, str.size())};
      size_t len {end - pos};
      bit_count = len;
      size_t byte_count {bytes_in_bits(len)};
      data.resize(byte_count, 0);

      for (size_t i = 0; i < len; ++i) {
        char c {str[pos + i]};
        bool bit;
        if (c == zero)
          bit = false;
        else if (c == one)
          bit = true;
        else
          throw std::invalid_argument("BitSet::BitSet: Invalid character in input string");

        set(i, bit);
      }
    }
    BitSet(const BitSet& other) = default;
    BitSet(BitSet&& other) noexcept = default;
    BitSet& operator=(const BitSet& other) = default;
    BitSet& operator=(BitSet&& other) noexcept = default;


    size_t size() const noexcept { return bit_count; }
    bool empty() const noexcept { return bit_count == 0; }


    bool operator[](size_t pos) const noexcept {
      size_t byte {pos / 8};
      size_t bit {pos % 8};
      return (data[byte] >> bit) & 1;
    }
    reference operator[](size_t pos) noexcept { return reference {this, pos}; }

    bool test(size_t pos) const {
      if (pos >= bit_count) throw std::out_of_range("BitSet::test: Position is out of range");
      return (*this)[pos];
    }


    BitSet& set() noexcept {
      std::fill(data.begin(), data.end(), 0xFF);
      sanitize_tail();
      return *this;
    }

    BitSet& reset() noexcept {
      std::fill(data.begin(), data.end(), 0);
      return *this;
    }

    BitSet& flip() noexcept {
      for (auto& byte : data) byte = ~byte;
      sanitize_tail();
      return *this;
    }

    BitSet& set(size_t pos, bool value = true) {
      if (pos >= bit_count) throw std::out_of_range("BitSet::set: Position is out of range");
      size_t byte {pos / 8};
      size_t bit {pos % 8};
      if (value)
        data[byte] |= (1 << bit);
      else
        data[byte] &= ~(1 << bit);
      return *this;
    }

    BitSet& reset(size_t pos) { return set(pos, false); }

    BitSet& flip(size_t pos) {
      if (pos >= bit_count) throw std::out_of_range("BitSet::flip: Position is out of range");
      size_t byte {pos / 8};
      size_t bit {pos % 8};
      data[byte] ^= (1 << bit);
      return *this;
    }


    bool all() const noexcept {
      if (bit_count == 0) return true;
      for (size_t i = 0; i < data.size() - 1; ++i) {
        if (data[i] != 0xFF) return false;
      }
      size_t tail_bits {bit_count % 8};
      bitStorage_t mask = tail_bits == 0 ? 0xFF : (1 << tail_bits) - 1;
      return (data.back() & mask) == mask;
    }

    bool any() const noexcept {
      for (const auto& byte : data)
        if (byte != 0) return true;
      return false;
    }

    bool none() const noexcept { return !any(); }

    size_t count() const noexcept {
      size_t total {0};
      for (const auto& byte : data) total += std::__popcount(byte);
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
      if (bit_count > sizeof(unsigned long long) * 8)
        throw std::overflow_error("BitSet::to_ullong: BitSet too large to convert");
      unsigned long long result {0};
      if (bit_count == 0) return result;
      std::memcpy(&result, data.data(), bytes_in_bits(bit_count));
      return result;
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
      if (this != &other) {
        std::swap(data, other.data);
        std::swap(bit_count, other.bit_count);
      }
    }


   private:
    size_t bit_count;
    std::vector<bitStorage_t> data;


    void sanitize_tail() noexcept {
      if (bit_count == 0) return;
      size_t tail_bits {bit_count % 8};
      if (tail_bits > 0) {
        bitStorage_t mask = (1 << tail_bits) - 1;
        data.back() &= mask;
      }
    }

    void check_size_match(const BitSet& other) const {
      if (bit_count != other.bit_count) throw std::invalid_argument("BitSet: Size mismatch between BitSets");
    }

    static size_t bytes_in_bits(size_t bits) noexcept { return (bits + 7) / 8; }

    static size_t get_length_inBits_ULL(unsigned long long val) noexcept {
      if (val == 0) {
        return 1;
      }
      size_t length {0};
      while (val > 0) {
        val >>= 1;
        ++length;
      }
      return length;
    }
  };


  inline BitSet operator&(const BitSet& a, const BitSet& b) {
    BitSet result(a);
    result &= b;
    return result;
  }

  inline BitSet operator|(const BitSet& a, const BitSet& b) {
    BitSet result(a);
    result |= b;
    return result;
  }

  inline BitSet operator^(const BitSet& a, const BitSet& b) {
    BitSet result(a);
    result ^= b;
    return result;
  }

  inline BitSet operator<<(const BitSet& a, size_t shift) {
    BitSet result(a);
    result <<= shift;
    return result;
  }

  inline BitSet operator>>(const BitSet& a, size_t shift) {
    BitSet result(a);
    result >>= shift;
    return result;
  }

  inline std::ostream& operator<<(std::ostream& os, const BitSet& bs) {
    return os << bs.to_string();
  }


  BitSet& BitSet::operator<<=(size_t shift) {
    if (bit_count == 0 || shift == 0) return *this;
    if (shift >= bit_count) {
      reset();
      return *this;
    }
    size_t byte_shift {shift / 8};
    size_t bit_shift {shift % 8};

    if (bit_shift == 0) {
      for (size_t i = data.size(); i-- > byte_shift;) data[i] = data[i - byte_shift];
      for (size_t i = 0; i < byte_shift; ++i) data[i] = 0;

    } else {
      for (size_t i = data.size(); i-- > byte_shift;) {
        bitStorage_t low = (i > byte_shift) ? data[i - byte_shift - 1] : 0;
        bitStorage_t high {data[i - byte_shift]};
        data[i] = (high << bit_shift) | (low >> (8 - bit_shift));
      }
      for (size_t i = 0; i < byte_shift; ++i) data[i] = 0;
      if (byte_shift < data.size()) data[byte_shift] &= (0xFF << bit_shift);
    }
    sanitize_tail();
    return *this;
  }

  BitSet& BitSet::operator>>=(size_t shift) {
    if (shift == 0 || bit_count == 0) return *this;
    if (shift >= bit_count) {
      reset();
      return *this;
    }
    size_t byte_shift {shift / 8};
    size_t bit_shift {shift % 8};

    if (bit_shift == 0) {
      for (size_t i = 0; i + byte_shift < data.size(); ++i) data[i] = data[i + byte_shift];
      for (size_t i = data.size() - byte_shift; i < data.size(); ++i) data[i] = 0;

    } else {
      for (size_t i = 0; i + byte_shift < data.size(); ++i) {
        uint8_t low = data[i + byte_shift] >> bit_shift;
        uint8_t high = (i + byte_shift + 1 < data.size()) ? data[i + byte_shift + 1] << (8 - bit_shift) : 0;
        data[i] = low | high;
      }
      for (size_t i = data.size() - byte_shift; i < data.size(); ++i) data[i] = 0;
    }
    sanitize_tail();
    return *this;
  }

}
