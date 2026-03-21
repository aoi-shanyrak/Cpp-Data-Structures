#include "headers/LinkedList.tpp"
#include <cassert>
#include <iostream>

using namespace aoi;

void test_singly_insert_erase() {
  std::cout << "Testing SinglyLinkedList insert_after/erase_after...\n";

  SinglyLinkedList<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(4);

  auto it = list.begin();
  ++it;
  it = list.insert_after(it, 3);

  assert(list.size() == 4);
  assert(list.back() == 4);

  int expected[] = {1, 2, 3, 4};
  int i = 0;
  for (auto val : list) {
    assert(val == expected[i++]);
  }
  it = list.begin();
  ++it;
  it = list.erase_after(it);

  assert(list.size() == 3);
  assert(*it == 4);

  int expected2[] = {1, 2, 4};
  i = 0;
  for (auto val : list) {
    assert(val == expected2[i++]);
  }

  std::cout << "  SinglyLinkedList insert_after/erase_after... passed\n";
}

void test_doubly_insert_erase() {
  std::cout << "Testing DoublyLinkedList insert/erase...\n";

  DoublyLinkedList<int> list;
  list.push_back(1);
  list.push_back(3);
  list.push_back(4);

  auto it = list.begin();
  ++it;
  it = list.insert(it, 2);

  assert(list.size() == 4);
  assert(*it == 2);

  int expected[] = {1, 2, 3, 4};
  int i = 0;
  for (auto val : list) {
    assert(val == expected[i++]);
  }

  it = list.begin();
  ++it;
  it = list.erase(it);

  assert(list.size() == 3);
  assert(*it == 3);

  int expected2[] = {1, 3, 4};
  i = 0;
  for (auto val : list) {
    assert(val == expected2[i++]);
  }

  std::cout << "  DoublyLinkedList insert/erase... passed\n";
}

void test_singly_boundaries() {
  std::cout << "Testing SinglyLinkedList boundary cases...\n";

  SinglyLinkedList<int> list;
  assert(list.begin() == list.end());

  auto end_it = list.end();
  auto ret = list.insert_after(end_it, 10);
  assert(ret == list.end());
  assert(list.size() == 0);

  ret = list.erase_after(end_it);
  assert(ret == list.end());
  assert(list.size() == 0);

  list.push_back(1);
  assert(list.size() == 1);
  assert(list.front() == 1);
  assert(list.back() == 1);

  auto first = list.begin();
  ret = list.erase_after(first);
  assert(ret == list.end());
  assert(list.size() == 1);
  assert(list.front() == 1);
  assert(list.back() == 1);

  ret = list.insert_after(first, 2);
  assert(list.size() == 2);
  assert(*ret == 2);
  assert(list.back() == 2);

  first = list.begin();
  ret = list.erase_after(first);
  assert(ret == list.end());
  assert(list.size() == 1);
  assert(list.front() == 1);
  assert(list.back() == 1);

  list.pop_back();
  assert(list.empty());
  assert(list.begin() == list.end());

  std::cout << "  SinglyLinkedList boundary cases... passed\n";
}

void test_doubly_boundaries() {
  std::cout << "Testing DoublyLinkedList boundary cases...\n";

  DoublyLinkedList<int> list;
  assert(list.begin() == list.end());

  auto end_it = list.end();
  auto ret = list.erase(end_it);
  assert(ret == list.end());
  assert(list.size() == 0);

  ret = list.insert(end_it, 10);
  assert(list.size() == 1);
  assert(*ret == 10);
  assert(list.front() == 10);
  assert(list.back() == 10);

  ret = list.erase(list.begin());
  assert(ret == list.end());
  assert(list.empty());

  list.push_back(1);
  list.push_back(2);
  list.push_back(3);
  assert(list.size() == 3);

  ret = list.erase(list.begin());
  assert(list.size() == 2);
  assert(*ret == 2);
  assert(list.front() == 2);

  auto it = list.begin();
  ++it;
  ret = list.erase(it);
  assert(ret == list.end());
  assert(list.size() == 1);
  assert(list.front() == 2);
  assert(list.back() == 2);

  list.pop_front();
  assert(list.empty());
  assert(list.begin() == list.end());

  std::cout << "  DoublyLinkedList boundary cases... passed\n";
}

void test_singly_basic() {
  std::cout << "Testing SinglyLinkedList basic operations...\n";

  SinglyLinkedList<int> list;
  list.push_back(10);
  list.push_back(20);
  list.push_front(5);

  assert(list.size() == 3);
  assert(list.front() == 5);
  assert(list.back() == 20);

  int expected[] = {5, 10, 20};
  int i = 0;
  for (auto val : list) {
    assert(val == expected[i++]);
  }

  std::cout << "  SinglyLinkedList basic operations... passed\n";
}

void test_doubly_basic() {
  std::cout << "Testing DoublyLinkedList basic operations...\n";

  DoublyLinkedList<int> list;
  list.push_back(10);
  list.push_back(20);
  list.push_front(5);

  assert(list.size() == 3);
  assert(list.front() == 5);
  assert(list.back() == 20);

  int expected[] = {5, 10, 20};
  int i = 0;
  for (auto val : list) {
    assert(val == expected[i++]);
  }

  std::cout << "  DoublyLinkedList basic operations... passed\n";
}

int main() {
  std::cout << "=== LinkedList Test Suite ===\n\n";

  try {
    test_singly_basic();
    test_doubly_basic();
    test_singly_insert_erase();
    test_doubly_insert_erase();
    test_singly_boundaries();
    test_doubly_boundaries();

    std::cout << "\n=== All tests passed! ===\n";
  } catch (const std::exception& e) {
    std::cerr << "Test failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
