#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "../headers/tree/Map.hpp"
#include "../headers/tree/Set.hpp"

using namespace aoi;

using IntMap = Map<int, std::string>;
using IntSet = Set<int>;

void assert_map_matches_model(const IntMap& m, const std::map<int, std::string>& model) {
  assert(m.size() == model.size());
  assert(m.empty() == model.empty());

  auto mit = model.begin();
  for (auto it = m.begin(); it != m.end(); ++it, ++mit) {
    assert(mit != model.end());
    assert(it->first == mit->first);
    assert(it->second == mit->second);
  }
  assert(mit == model.end());

  auto mrit = model.rbegin();
  for (auto rit = m.rbegin(); rit != m.rend(); ++rit, ++mrit) {
    assert(mrit != model.rend());
    assert(rit->first == mrit->first);
    assert(rit->second == mrit->second);
  }
  assert(mrit == model.rend());
}

void assert_set_matches_model(const IntSet& s, const std::set<int>& model) {
  assert(s.size() == model.size());
  assert(s.empty() == model.empty());

  auto sit = model.begin();
  for (auto it = s.begin(); it != s.end(); ++it, ++sit) {
    assert(sit != model.end());
    assert(*it == *sit);
  }
  assert(sit == model.end());

  auto srit = model.rbegin();
  for (auto rit = s.rbegin(); rit != s.rend(); ++rit, ++srit) {
    assert(srit != model.rend());
    assert(*rit == *srit);
  }
  assert(srit == model.rend());
}

void test_map_constructors_and_assignment() {
  std::cout << "Testing Map constructors and assignment... ";

  IntMap m_default;
  assert(m_default.empty());
  assert(m_default.max_size() > 0);

  std::vector<std::pair<int, std::string>> seed {{3, "three"}, {1, "one"}, {2, "two"}};
  IntMap m_range(seed.begin(), seed.end());
  std::map<int, std::string> model_range {{1, "one"}, {2, "two"}, {3, "three"}};
  assert_map_matches_model(m_range, model_range);

  IntMap m_init {{std::pair<const int, std::string> {7, "seven"}, std::pair<const int, std::string> {4, "four"}}};
  std::map<int, std::string> model_init {{4, "four"}, {7, "seven"}};
  assert_map_matches_model(m_init, model_init);

  IntMap m_copy {m_range};
  assert(m_copy == m_range);

  IntMap m_move {std::move(m_copy)};
  assert(m_move == m_range);

  IntMap m_assign;
  m_assign = m_move;
  assert(m_assign == m_move);

  IntMap m_move_assign;
  m_move_assign = std::move(m_assign);
  assert(m_move_assign == m_move);

  m_move_assign = {{std::pair<const int, std::string> {9, "nine"}, std::pair<const int, std::string> {5, "five"}}};
  std::map<int, std::string> model_assign {{5, "five"}, {9, "nine"}};
  assert_map_matches_model(m_move_assign, model_assign);

  std::cout << "passed\n";
}

void test_map_all_modifiers_and_lookup() {
  std::cout << "Testing Map modifiers and lookup... ";

  IntMap m;
  std::map<int, std::string> model;

  auto [it1, inserted1] = m.insert(std::pair<const int, std::string> {3, "three"});
  assert(inserted1);
  assert(it1->first == 3);
  model[3] = "three";

  auto hint = m.begin();
  auto it2 = m.insert(hint, std::pair<const int, std::string> {1, "one"});
  assert(it2->first == 1);
  model[1] = "one";

  std::vector<std::pair<int, std::string>> extra {{5, "five"}, {2, "two"}};
  m.insert(extra.begin(), extra.end());
  model[5] = "five";
  model[2] = "two";

  m.insert({std::pair<const int, std::string> {4, "four"}, std::pair<const int, std::string> {3, "THREE"}});
  model[4] = "four";
  model[3] = "THREE";

  auto [eit, eins] = m.emplace(6, "six");
  assert(eins);
  assert(eit->first == 6);
  model[6] = "six";

  auto ehit = m.emplace_hint(m.begin(), 7, "seven");
  assert(ehit->first == 7);
  model[7] = "seven";

  m[8] = "eight";
  model[8] = "eight";
  m[3] = "three-updated";
  model[3] = "three-updated";

  assert(m.at(8) == "eight");
  bool threw = false;
  try {
    (void)m.at(999);
  } catch (const std::out_of_range&) {
    threw = true;
  }
  assert(threw);

  assert(m.contains(7));
  assert(!m.contains(100));
  assert(m.count(7) == 1);
  assert(m.count(100) == 0);

  auto f = m.find(5);
  assert(f != m.end() && f->second == "five");
  const IntMap& cm = m;
  auto cf = cm.find(5);
  assert(cf != cm.end() && cf->second == "five");

  assert(m.erase(-10) == 0);
  assert(m.erase(1) == 1);
  model.erase(1);

  auto fit = m.find(2);
  assert(fit != m.end());
  auto next = m.erase(fit);
  model.erase(2);
  if (next != m.end()) assert(next->first > 2);

  auto cfit = cm.find(3);
  auto after_const_erase = m.erase(cfit);
  model.erase(3);
  if (after_const_erase != m.end()) assert(after_const_erase->first > 3);

  auto first = m.find(4);
  auto last = m.find(7);
  auto after_range = m.erase(first, last);
  model.erase(4);
  model.erase(5);
  model.erase(6);
  assert(after_range != m.end());
  assert(after_range->first == 7);

  assert_map_matches_model(m, model);

  m.clear();
  model.clear();
  assert_map_matches_model(m, model);

  std::cout << "passed\n";
}

void test_map_swap_and_relational() {
  std::cout << "Testing Map swap and relational operators... ";

  IntMap a {{std::pair<const int, std::string> {1, "a"}, std::pair<const int, std::string> {3, "c"}}};
  IntMap b {{std::pair<const int, std::string> {2, "b"}}};
  IntMap c {{std::pair<const int, std::string> {2, "b"}}};

  assert(b == c);
  assert(!(b != c));
  assert(a != b);

  assert(b <= c);
  assert(b >= c);
  assert(b < a || a < b);

  a.swap(b);
  assert(a.size() == 1 && a.contains(2));
  assert(b.size() == 2 && b.contains(1) && b.contains(3));

  std::cout << "passed\n";
}

void test_set_constructors_and_assignment() {
  std::cout << "Testing Set constructors and assignment... ";

  IntSet s_default;
  assert(s_default.empty());
  assert(s_default.max_size() > 0);

  std::vector<int> seed {4, 1, 3, 1};
  IntSet s_range(seed.begin(), seed.end());
  std::set<int> model_range {1, 3, 4};
  assert_set_matches_model(s_range, model_range);

  IntSet s_init {7, 2, 9, 2};
  std::set<int> model_init {2, 7, 9};
  assert_set_matches_model(s_init, model_init);

  IntSet s_copy {s_range};
  assert(s_copy == s_range);

  IntSet s_move {std::move(s_copy)};
  assert(s_move == s_range);

  IntSet s_assign;
  s_assign = s_move;
  assert(s_assign == s_move);

  IntSet s_move_assign;
  s_move_assign = std::move(s_assign);
  assert(s_move_assign == s_move);

  s_move_assign = {10, 5, 10};
  std::set<int> model_assign {5, 10};
  assert_set_matches_model(s_move_assign, model_assign);

  std::cout << "passed\n";
}

void test_set_all_modifiers_and_lookup() {
  std::cout << "Testing Set modifiers and lookup... ";

  IntSet s;
  std::set<int> model;

  auto [it1, inserted1] = s.insert(3);
  assert(inserted1 && *it1 == 3);
  model.insert(3);

  auto it2 = s.insert(s.begin(), 1);
  assert(*it2 == 1);
  model.insert(1);

  std::vector<int> extra {5, 2, 2};
  s.insert(extra.begin(), extra.end());
  model.insert(5);
  model.insert(2);

  s.insert({4, 6, 4});
  model.insert(4);
  model.insert(6);

  auto [eit, eins] = s.emplace(7);
  assert(eins && *eit == 7);
  model.insert(7);

  auto ehit = s.emplace_hint(s.begin(), 8);
  assert(*ehit == 8);
  model.insert(8);

  assert(s.contains(8));
  assert(!s.contains(100));
  assert(s.count(8) == 1);
  assert(s.count(100) == 0);

  auto f = s.find(5);
  assert(f != s.end() && *f == 5);
  const IntSet& cs = s;
  auto cf = cs.find(5);
  assert(cf != cs.end() && *cf == 5);

  assert(s.erase(-10) == 0);
  assert(s.erase(1) == 1);
  model.erase(1);

  auto fit = s.find(2);
  assert(fit != s.end());
  auto next = s.erase(fit);
  model.erase(2);
  if (next != s.end()) assert(*next > 2);

  auto first = s.find(4);
  auto last = s.find(7);
  auto after_range = s.erase(first, last);
  model.erase(4);
  model.erase(5);
  model.erase(6);
  assert(after_range != s.end());
  assert(*after_range == 7);

  assert_set_matches_model(s, model);

  s.clear();
  model.clear();
  assert_set_matches_model(s, model);

  std::cout << "passed\n";
}

void test_set_swap_and_relational() {
  std::cout << "Testing Set swap and relational operators... ";

  IntSet a {1, 3};
  IntSet b {2};
  IntSet c {2};

  assert(b == c);
  assert(!(b != c));
  assert(a != b);

  assert(b <= c);
  assert(b >= c);
  assert(b < a || a < b);

  a.swap(b);
  assert(a.size() == 1 && a.contains(2));
  assert(b.size() == 2 && b.contains(1) && b.contains(3));

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== Map/Set Comprehensive Test Suite ===\n\n";

  try {
    test_map_constructors_and_assignment();
    test_map_all_modifiers_and_lookup();
    test_map_swap_and_relational();
    test_set_constructors_and_assignment();
    test_set_all_modifiers_and_lookup();
    test_set_swap_and_relational();

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
