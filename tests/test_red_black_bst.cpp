#include <cassert>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <vector>

#include "../headers/tree/RedBlackBST.tpp"

using namespace aoi;

using RB = RedBlackBST<int, int>;

void assert_find_throws(const RB& tree, int key) {
  bool threw = false;
  try {
    (void)tree.find(key);
  } catch (const std::out_of_range&) {
    threw = true;
  }
  assert(threw);
}

void assert_tree_matches_model(RB& tree, const std::map<int, int>& model) {
  assert(tree.empty() == model.empty());
  assert(tree.size() == model.size());

  if (model.empty()) {
    return;
  }

  assert(tree.min() == model.begin()->second);
  assert(tree.max() == model.rbegin()->second);

  assert(tree.succ(RB::nullindex) == RB::nullindex);
  assert(tree.pred(RB::nullindex) == RB::nullindex);

  for (auto it = model.begin(); it != model.end(); ++it) {
    int key = it->first;
    int value = it->second;

    auto h = tree.search(key);
    assert(h != RB::nullindex);
    assert(tree.find(key) == value);
    assert(tree.get_by_handle(h) == value);

    auto next_it = std::next(it);
    auto hs = tree.succ(h);
    if (next_it == model.end()) {
      assert(hs == RB::nullindex);
    } else {
      auto expected = tree.search(next_it->first);
      assert(expected != RB::nullindex);
      assert(hs == expected);
      assert(tree.get_by_handle(hs) == next_it->second);
    }

    auto hp = tree.pred(h);
    if (it == model.begin()) {
      assert(hp == RB::nullindex);
    } else {
      auto prev_it = std::prev(it);
      auto expected = tree.search(prev_it->first);
      assert(expected != RB::nullindex);
      assert(hp == expected);
      assert(tree.get_by_handle(hp) == prev_it->second);
    }
  }
}

void test_basic_insert_find_update_remove() {
  std::cout << "Testing basic insert/find/update/remove... ";

  RB tree;
  assert(tree.empty());
  assert(tree.size() == 0);

  tree.insert(10, 100);
  tree.insert(5, 50);
  tree.insert(15, 150);

  assert(!tree.empty());
  assert(tree.size() == 3);

  assert(tree.find(10) == 100);
  assert(tree.find(5) == 50);
  assert(tree.find(15) == 150);

  tree.insert(10, 101);
  assert(tree.size() == 3);
  assert(tree.find(10) == 101);

  tree.remove(42);
  assert(tree.size() == 3);

  tree.remove(10);
  assert(tree.size() == 2);

  assert_find_throws(tree, 10);

  assert(tree.find(5) == 50);
  assert(tree.find(15) == 150);

  std::cout << "passed\n";
}

void test_search_handle_and_succ_pred_api() {
  std::cout << "Testing search/get_by_handle/succ/pred API... ";

  RB tree;
  std::map<int, int> model;

  const std::vector<int> keys = {20, 10, 30, 5, 15, 25, 35, 13, 27};
  for (int k : keys) {
    tree.insert(k, k * 10);
    model[k] = k * 10;
  }

  assert_tree_matches_model(tree, model);

  // Delete a node with two children, then with one/zero child patterns.
  tree.remove(10);
  model.erase(10);
  tree.remove(27);
  model.erase(27);
  tree.remove(30);
  model.erase(30);

  assert_tree_matches_model(tree, model);

  // Update existing keys and re-check handles/neighbor relationships.
  tree.insert(25, 2500);
  model[25] = 2500;
  tree.insert(5, 5000);
  model[5] = 5000;

  assert_tree_matches_model(tree, model);

  auto miss = tree.search(999);
  assert(miss == RB::nullindex);
  assert_find_throws(tree, 999);

  std::cout << "passed\n";
}

void test_clear_and_reuse() {
  std::cout << "Testing clear and reuse... ";

  RB tree;

  for (int round = 0; round < 40; ++round) {
    std::map<int, int> model;

    for (int k = -120; k <= 120; ++k) {
      int v = round * 100000 + k;
      tree.insert(k, v);
      model[k] = v;
    }
    assert_tree_matches_model(tree, model);

    tree.clear();
    assert(tree.empty());
    assert(tree.size() == 0);

    for (int probe : {-120, -1, 0, 1, 120}) {
      assert(tree.search(probe) == RB::nullindex);
      assert_find_throws(tree, probe);
    }

    for (int k = 0; k < 25; ++k) {
      tree.insert(k, k + round);
    }
    for (int k = 0; k < 25; ++k) {
      assert(tree.find(k) == k + round);
    }
    tree.clear();
  }

  std::cout << "passed\n";
}

void test_remove_all_orders() {
  std::cout << "Testing full removals in different orders... ";

  {
    RB tree;
    std::map<int, int> model;
    for (int k = 0; k < 300; ++k) {
      tree.insert(k, k * 7);
      model[k] = k * 7;
    }
    assert_tree_matches_model(tree, model);

    for (int k = 0; k < 300; ++k) {
      tree.remove(k);
      model.erase(k);
      if (k % 17 == 0) {
        assert_tree_matches_model(tree, model);
      }
    }
    assert(tree.empty());
    assert(tree.size() == 0);
  }

  {
    RB tree;
    std::map<int, int> model;
    for (int k = 0; k < 300; ++k) {
      tree.insert(k, -k);
      model[k] = -k;
    }
    assert_tree_matches_model(tree, model);

    for (int k = 299; k >= 0; --k) {
      tree.remove(k);
      model.erase(k);
      if (k % 19 == 0) {
        assert_tree_matches_model(tree, model);
      }
    }
    assert(tree.empty());
    assert(tree.size() == 0);
  }

  std::cout << "passed\n";
}

void test_randomized_against_std_map() {
  std::cout << "Testing long randomized operations against std::map... ";

  RB tree;
  std::map<int, int> model;

  std::mt19937 rng(123456789);
  std::uniform_int_distribution<int> op_dist(0, 4);
  std::uniform_int_distribution<int> key_dist(-400, 400);
  std::uniform_int_distribution<int> val_dist(-100000, 100000);

  for (int i = 0; i < 120000; ++i) {
    int op = op_dist(rng);
    int key = key_dist(rng);

    if (op <= 1) {
      int value = val_dist(rng);
      tree.insert(key, value);
      model[key] = value;

    } else if (op == 2) {
      tree.remove(key);
      model.erase(key);

    } else if (op == 3) {
      auto it = model.find(key);
      if (it == model.end()) {
        assert_find_throws(tree, key);
      } else {
        assert(tree.find(key) == it->second);
      }

    } else {
      // Deterministic updates for existing and non-existing keys.
      int value = key * 17 + i;
      tree.insert(key, value);
      model[key] = value;
      assert(tree.find(key) == value);
    }

    // Frequent consistency checks around key range.
    if (i % 13 == 0) {
      for (int probe = -420; probe <= 420; probe += 35) {
        auto it = model.find(probe);
        auto h = tree.search(probe);
        if (it == model.end()) {
          assert(h == RB::nullindex);
          assert_find_throws(tree, probe);
        } else {
          assert(h != RB::nullindex);
          assert(tree.get_by_handle(h) == it->second);
          assert(tree.find(probe) == it->second);
        }
      }
    }

    if (i % 97 == 0) {
      assert_tree_matches_model(tree, model);
    }
  }

  assert_tree_matches_model(tree, model);

  // Final destructive phase in randomized key order.
  std::vector<int> keys;
  keys.reserve(model.size());
  for (const auto& [k, _] : model) {
    (void)_;
    keys.push_back(k);
  }
  std::shuffle(keys.begin(), keys.end(), rng);

  for (size_t i = 0; i < keys.size(); ++i) {
    int k = keys[i];
    tree.remove(k);
    model.erase(k);
    if (i % 23 == 0) {
      assert_tree_matches_model(tree, model);
    }
  }

  assert(tree.empty());
  assert(tree.size() == 0);

  std::cout << "passed\n";
}

int main() {
  std::cout << "=== RedBlackBST Test Suite ===\n\n";

  test_basic_insert_find_update_remove();
  test_search_handle_and_succ_pred_api();
  test_clear_and_reuse();
  test_remove_all_orders();
  test_randomized_against_std_map();

  std::cout << "\n=== RedBlackBST tests passed! ===\n";
  return 0;
}
