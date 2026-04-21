#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../headers/utility/BitSet.hpp"

using namespace aoi;

void test_default_constructor() {
  std::cout << "Testing default constructor... ";
  BitSet bs;
  assert(bs.size() == 0);
  assert(bs.empty());
  assert(bs.none());
  assert(bs.all());
  assert(bs.count() == 0);
  assert(bs.to_string().empty());
  assert(bs.to_ullong() == 0);
  std::cout << "passed\n";
}

void test_size_constructor() {
  std::cout << "Testing size constructor... ";
  BitSet bs(static_cast<size_t>(10));
  assert(bs.size() == 10);
  assert(!bs.empty());
  assert(bs.none());
  assert(!bs.any());
  assert(!bs.all());
  assert(bs.count() == 0);
  assert(bs.to_string() == std::string(10, '0'));
  std::cout << "passed\n";
}

void test_ull_constructor() {
  std::cout << "Testing unsigned long long constructor... ";
  BitSet bs(0b101101ull);
  assert(bs.size() == 6);
  assert(bs.to_string() == "101101");
  assert(bs.to_ullong() == 0b101101u);
  assert(bs.count() == 4);
  std::cout << "passed\n";
}

void test_string_constructor() {
  std::cout << "Testing string constructor... ";
  BitSet bs(std::string {"xoxxo"}, 1, 3, 'x', 'o');
  assert(bs.size() == 3);
  assert(bs.to_string('x', 'o') == "oxx");
  assert(bs.count() == 1);
  assert(bs.test(0));
  assert(!bs.test(1));
  assert(!bs.test(2));
  std::cout << "passed\n";
}

void test_copy_and_move() {
  std::cout << "Testing copy and move... ";
  BitSet original(0b1101ull);
  BitSet copied(original);
  BitSet moved(std::move(copied));
  assert(original == moved);
  BitSet assigned(0b0000ull);
  assigned = original;
  assert(assigned == original);
  BitSet move_assigned(0b1ull);
  move_assigned = std::move(assigned);
  assert(move_assigned == original);
  std::cout << "passed\n";
}

void test_element_access_and_reference() {
  std::cout << "Testing element access and reference proxy... ";
  BitSet bs(static_cast<size_t>(8));
  bs[1] = true;
  bs[3] = true;
  assert(bs.test(1));
  assert(bs.test(3));
  assert(!bs.test(0));
  bs[3].flip();
  assert(!bs.test(3));
  bs[1] = bs[3];
  assert(!bs.test(1));
  assert(bs[0] == false);
  bs.set(0);
  assert(bs[0]);
  bs.reset(0);
  assert(!bs[0]);
  bs.flip(0);
  assert(bs[0]);
  std::cout << "passed\n";
}

void test_bulk_modifiers() {
  std::cout << "Testing bulk modifiers... ";
  BitSet bs(static_cast<size_t>(10));
  bs.set();
  assert(bs.count() == 10);
  assert(bs.all());
  bs.flip();
  assert(bs.none());
  bs.set(4, true).set(9, true);
  assert(bs.count() == 2);
  bs.reset();
  assert(bs.none());
  std::cout << "passed\n";
}

void test_bitwise_operations() {
  std::cout << "Testing bitwise operations... ";
  BitSet a(std::string {"10101100"});
  BitSet b(std::string {"11001010"});

  BitSet and_result = a & b;
  BitSet or_result = a | b;
  BitSet xor_result = a ^ b;
  BitSet not_result = ~a;

  assert(and_result.to_string() == "10001000");
  assert(or_result.to_string() == "11101110");
  assert(xor_result.to_string() == "01100110");
  assert(not_result.to_string() == "01010011");

  a &= b;
  assert(a == and_result);
  b |= xor_result;
  assert(b.to_string() == "11101110");
  std::cout << "passed\n";
}

void test_shift_operations() {
  std::cout << "Testing shift operations... ";
  BitSet left(std::string {"10110011"});
  left <<= 3;
  assert(left.to_string() == "00010110");

  BitSet right(std::string {"10110011"});
  right >>= 2;
  assert(right.to_string() == "11001100");

  BitSet cleared_left(std::string {"1011"});
  cleared_left <<= 10;
  assert(cleared_left.none());

  BitSet cleared_right(std::string {"1011"});
  cleared_right >>= 10;
  assert(cleared_right.none());
  std::cout << "passed\n";
}

void test_to_ullong_and_string_output() {
  std::cout << "Testing string and integer conversion... ";
  BitSet bs(std::string {"10000001"});
  std::ostringstream out;
  out << bs;
  assert(out.str() == "10000001");
  assert(bs.to_ullong() == 0b10000001u);
  assert(bs.to_string('F', 'T') == "TFFFFFFT");
  std::cout << "passed\n";
}

void test_swap_and_comparisons() {
  std::cout << "Testing swap and comparisons... ";
  BitSet a(std::string {"1010"});
  BitSet b(std::string {"0101"});
  a.swap(b);
  assert(a.to_string() == "0101");
  assert(b.to_string() == "1010");
  assert(a != b);
  BitSet c(std::string {"0101"});
  assert(a == c);
  std::cout << "passed\n";
}

void test_exceptions() {
  std::cout << "Testing exceptions... ";
  bool threw = false;

  try {
    BitSet bs(std::string {"0102"});
    (void)bs;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    BitSet bs(std::string {"01"}, 5);
    (void)bs;
  } catch (const std::out_of_range&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    BitSet a(static_cast<size_t>(3));
    BitSet b(static_cast<size_t>(4));
    a &= b;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    BitSet bs(static_cast<size_t>(65));
    (void)bs.to_ullong();
  } catch (const std::overflow_error&) {
    threw = true;
  }
  assert(threw);

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== BitSet Test Suite ===\n\n";

  try {
    test_default_constructor();
    test_size_constructor();
    test_ull_constructor();
    test_string_constructor();
    test_copy_and_move();
    test_element_access_and_reference();
    test_bulk_modifiers();
    test_bitwise_operations();
    test_shift_operations();
    test_to_ullong_and_string_output();
    test_swap_and_comparisons();
    test_exceptions();

    std::cout << "\n=== All tests passed! ===\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "\nTest failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\nTest failed with unknown exception\n";
    return 1;
  }
}
