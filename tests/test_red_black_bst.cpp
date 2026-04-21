#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <vector>

#include "../headers/tree/impl/RedBlackBST.tpp"

using namespace aoi;

using RB = RedBlackBST<int, int>;

void assert_iterator_order(const RB& tree, const std::map<int, int>& model) {
  auto mit = model.begin();
  for (auto it = tree.cbegin(); it != tree.cend(); ++it, ++mit) {
    assert(mit != model.end());
    assert(it->first == mit->first);
    assert(it->second == mit->second);
    assert(it.handle() == tree.search_handle(mit->first));
  }
  assert(mit == model.end());

  if (model.empty()) {
    assert(tree.cbegin() == tree.cend());
    return;
  }

  auto rit = model.rbegin();
  for (auto it = tree.cend(); it != tree.cbegin();) {
    --it;
    assert(rit != model.rend());
    assert(it->first == rit->first);
    assert(it->second == rit->second);
    assert(it.handle() == tree.search_handle(rit->first));
    ++rit;
  }
  assert(rit == model.rend());
}

void assert_tree_matches_model(RB& tree, const std::map<int, int>& model) {
  assert(tree.empty() == model.empty());
  assert(tree.size() == model.size());
  assert(tree.succ(RB::nullindex) == RB::nullindex);
  assert(tree.pred(RB::nullindex) == RB::nullindex);

  assert_iterator_order(tree, model);

  if (model.empty()) {
    return;
  }

  assert(tree.min() == model.begin()->second);
  assert(tree.max() == model.rbegin()->second);

  auto mit = model.begin();
  for (auto it = tree.begin(); it != tree.end(); ++it, ++mit) {
    assert(mit != model.end());
    assert(it->first == mit->first);
    assert(it->second == mit->second);
    assert(it.handle() == tree.search_handle(mit->first));

    auto found = tree.find(mit->first);
    assert(found != tree.end());
    assert(found.handle() == tree.search_handle(mit->first));
    assert(found->first == mit->first);
    assert(found->second == mit->second);
    assert(tree.contains(mit->first));
    assert(tree.at(mit->first) == mit->second);
    assert(tree.get_by_handle(found.handle()) == mit->second);

    auto h = tree.search_handle(mit->first);
    assert(h != RB::nullindex);
    assert(tree.get_by_handle(h) == mit->second);

    auto hs = tree.succ(h);
    auto next = std::next(mit);
    if (next == model.end()) {
      assert(hs == RB::nullindex);
    } else {
      auto expected = tree.search_handle(next->first);
      assert(expected != RB::nullindex);
      assert(hs == expected);
      assert(tree.get_by_handle(hs) == next->second);
    }

    auto hp = tree.pred(h);
    if (mit == model.begin()) {
      assert(hp == RB::nullindex);
    } else {
      auto prev = std::prev(mit);
      auto expected = tree.search_handle(prev->first);
      assert(expected != RB::nullindex);
      assert(hp == expected);
      assert(tree.get_by_handle(hp) == prev->second);
    }
  }
  assert(mit == model.end());
}

void test_insert_find_contains_and_index_operator() {
  std::cout << "Testing insert/find/contains/operator[]... ";

  RB tree;
  std::map<int, int> model;

  assert(tree.empty());
  assert(tree.begin() == tree.end());
  assert(tree.cbegin() == tree.cend());

  auto [it1, inserted1] = tree.insert(10, 100);
  assert(inserted1);
  assert(it1.handle() == tree.search_handle(10));
  model[10] = 100;

  auto [it2, inserted2] = tree.insert(4, 40);
  assert(inserted2);
  assert(it2.handle() == tree.search_handle(4));
  model[4] = 40;

  auto [it3, inserted3] = tree.insert(17, 170);
  assert(inserted3);
  assert(it3.handle() == tree.search_handle(17));
  model[17] = 170;

  auto [it4, inserted4] = tree.insert(10, 111);
  assert(!inserted4);
  assert(it4.handle() == tree.search_handle(10));
  model[10] = 111;

  assert(tree.contains(4));
  assert(!tree.contains(999));
  assert(tree.at(10) == 111);
  assert(tree.get_by_handle(tree.search_handle(10)) == 111);
  assert(tree.find(999) == tree.end());

  tree[99] = 9900;
  model[99] = 9900;
  assert(tree.at(99) == 9900);
  assert(tree.get_by_handle(tree.search_handle(99)) == 9900);

  tree[10] += 9;
  model[10] += 9;
  assert(tree.at(10) == 120);
  assert(tree.get_by_handle(tree.search_handle(10)) == 120);

  assert_tree_matches_model(tree, model);

  std::cout << "passed\n";
}

void test_erase_by_key_and_iterator_variants() {
  std::cout << "Testing erase by key/iterator/const_iterator... ";

  RB tree;
  std::map<int, int> model;

  for (int k = 0; k < 25; ++k) {
    tree.insert(k, k * 10);
    model[k] = k * 10;
  }
  assert_tree_matches_model(tree, model);

  assert(tree.erase(-100) == 0);

  auto it = tree.find(0);
  assert(it != tree.end());
  auto next = tree.erase(it);
  model.erase(0);
  assert(next != tree.end());
  assert(next.handle() == tree.search_handle(1));
  assert_tree_matches_model(tree, model);

  auto cit = static_cast<const RB&>(tree).find(12);
  assert(cit != tree.cend());
  auto after = tree.erase(cit);
  model.erase(12);
  assert(after != tree.end());
  assert(after.handle() == tree.search_handle(13));
  assert_tree_matches_model(tree, model);

  auto last = tree.find(24);
  assert(last != tree.end());
  auto end_after = tree.erase(last);
  model.erase(24);
  assert(end_after == tree.end());

  assert(tree.erase(8) == 1);
  model.erase(8);
  assert(tree.erase(8) == 0);

  assert_tree_matches_model(tree, model);

  std::cout << "passed\n";
}

void test_copy_move_clear_and_reuse() {
  std::cout << "Testing copy/move/clear/reuse semantics... ";

  RB tree;
  std::map<int, int> model;
  for (int k = -40; k <= 40; ++k) {
    tree.insert(k, k * 3);
    model[k] = k * 3;
  }

  RB copy_ctor {tree};
  assert_tree_matches_model(copy_ctor, model);

  tree[0] = 777;
  model[0] = 777;
  assert(copy_ctor.at(0) == 0);
  assert(copy_ctor.get_by_handle(copy_ctor.search_handle(0)) == 0);

  RB copy_assign;
  copy_assign = tree;
  assert_tree_matches_model(copy_assign, model);

  RB move_ctor {std::move(copy_assign)};
  assert_tree_matches_model(move_ctor, model);

  RB move_assign;
  move_assign = std::move(move_ctor);
  assert_tree_matches_model(move_assign, model);

  tree.clear();
  assert(tree.empty());
  assert(tree.size() == 0);
  assert(tree.begin() == tree.end());

  for (int round = 0; round < 5; ++round) {
    std::map<int, int> local;
    for (int k = -20; k <= 20; ++k) {
      tree.insert(k, round * 100 + k);
      local[k] = round * 100 + k;
    }
    assert_tree_matches_model(tree, local);
    tree.clear();
    assert(tree.empty());
  }

  std::cout << "passed\n";
}

void test_shrink_to_fit_keeps_contents_and_order() {
  std::cout << "Testing shrink_to_fit stability... ";

  RB tree;
  std::map<int, int> model;

  for (int k = -150; k <= 150; ++k) {
    tree.insert(k, k * k - 7);
    model[k] = k * k - 7;
  }

  for (int k = -150; k <= 150; k += 3) {
    tree.erase(k);
    model.erase(k);
  }

  assert_tree_matches_model(tree, model);

  tree.shrink_to_fit();
  assert_tree_matches_model(tree, model);

  for (int k = -149; k <= 149; k += 5) {
    tree[k] += 1;
    model[k] += 1;
  }

  tree.shrink_to_fit();
  assert_tree_matches_model(tree, model);

  std::cout << "passed\n";
}

void run_randomized_against_std_map(int operations, uint32_t seed) {
  RB tree;
  std::map<int, int> model;
  std::mt19937 rng(seed);

  std::uniform_int_distribution<int> op_dist(0, 8);
  std::uniform_int_distribution<int> key_dist(-500, 500);
  std::uniform_int_distribution<int> val_dist(-200000, 200000);

  for (int i = 1; i <= operations; ++i) {
    int op = op_dist(rng);
    int key = key_dist(rng);

    if (op <= 2) {
      int value = val_dist(rng);
      auto [it, inserted] = tree.insert(key, value);
      (void)it;
      (void)inserted;
      model[key] = value;

    } else if (op == 3) {
      size_t erased = tree.erase(key);
      size_t model_erased = model.erase(key);
      assert(erased == model_erased);

    } else if (op == 4) {
      int value = val_dist(rng);
      tree[key] = value;
      model[key] = value;

    } else if (op == 5) {
      auto it = tree.find(key);
      auto mit = model.find(key);
      if (mit == model.end()) {
        assert(it == tree.end());
        assert(!tree.contains(key));
      } else {
        assert(it != tree.end());
        assert(it->first == mit->first);
        assert(it->second == mit->second);
        assert(it.handle() == tree.search_handle(mit->first));
        assert(tree.get_by_handle(it.handle()) == mit->second);
        assert(tree.contains(key));
        assert(tree.at(key) == mit->second);
      }

    } else if (op == 6) {
      if (!model.empty()) {
        auto mit = model.begin();
        std::advance(mit, static_cast<long>(rng() % model.size()));
        auto it = tree.find(mit->first);
        assert(it != tree.end());
        auto next = tree.erase(it);
        model.erase(mit);

        if (!model.empty() && next != tree.end()) {
          bool found_match = false;
          for (const auto& [k, _] : model) {
            (void)_;
            if (tree.search_handle(k) == next.handle()) {
              found_match = true;
              break;
            }
          }
          assert(found_match);
        }
      }

    } else if (op == 7) {
      tree.shrink_to_fit();

    } else {
      if (!model.empty()) {
        auto first_h = tree.search_handle(model.begin()->first);
        auto last_h = tree.search_handle(model.rbegin()->first);
        assert(tree.pred(first_h) == RB::nullindex);
        assert(tree.succ(last_h) == RB::nullindex);
      }
    }

    if (i % 200 == 0) {
      assert_tree_matches_model(tree, model);
    }
  }

  assert_tree_matches_model(tree, model);

  std::vector<int> keys;
  keys.reserve(model.size());
  for (const auto& [k, _] : model) {
    (void)_;
    keys.push_back(k);
  }
  std::shuffle(keys.begin(), keys.end(), rng);

  for (size_t i = 0; i < keys.size(); ++i) {
    assert(tree.erase(keys[i]) == 1);
    if (i % 37 == 0) {
      assert(tree.succ(RB::nullindex) == RB::nullindex);
      assert(tree.pred(RB::nullindex) == RB::nullindex);
    }
  }

  assert(tree.empty());
  assert(tree.size() == 0);
}


int main() {
  std::cout << "=== RedBlackBST Extended Test Suite ===\n\n";

  test_insert_find_contains_and_index_operator();
  test_erase_by_key_and_iterator_variants();
  test_copy_move_clear_and_reuse();
  test_shrink_to_fit_keeps_contents_and_order();

#ifdef DEEP_TEST
  std::cout << "Running deep randomized pass... ";
  run_randomized_against_std_map(200000, 0x13579BDFu);
  std::cout << "passed\n";
#else
  std::cout << "Skipping deep randomized pass (build with -DDEEP_TEST to enable).\n";
#endif

  std::cout << "\n=== RedBlackBST extended tests passed! ===\n";
  return 0;
}
