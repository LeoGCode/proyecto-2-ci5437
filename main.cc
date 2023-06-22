// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by:

#include <iostream>
#include <limits>
#include <unordered_map>

#include "othello_cut.h"  // won't work correctly until .h is fixed!
#include "utils.h"

using namespace std;

unsigned expanded = 0;
unsigned generated = 0;
int tt_threshold = 32;  // threshold to save entries in TT

// Transposition table (it is not necessary to implement TT)
struct stored_info_t {
  int value_;
  int type_;
  enum { EXACT, LOWER, UPPER };
  stored_info_t(int value = -100, int type = LOWER)
      : value_(value), type_(type) {}
};

struct hash_function_t {
  size_t operator()(const state_t &state) const { return state.hash(); }
};

class hash_table_t
    : public unordered_map<state_t, stored_info_t, hash_function_t> {};

hash_table_t TTable[2];

// int maxmin(state_t state, int depth, bool use_tt);
// int minmax(state_t state, int depth, bool use_tt = false);
// int maxmin(state_t state, int depth, bool use_tt = false);

int negamax(state_t state, int depth, int color, bool use_tt = false);
int negamax(state_t state, int depth, int alpha, int beta, int color,
            bool use_tt = false);
int scout(state_t state, int depth, int color, bool use_tt = false);
int negascout(state_t state, int depth, int alpha, int beta, int color,
              bool use_tt = false);

/**
 * Negamax algorithm
 * @param state current state
 * @param depth current depth
 * @param color current player, 1 for MAX, -1 for MIN
 * @param use_tt use transposition table
 * @return value of the state
 */
int negamax(state_t state, int depth, int color, bool use_tt) {
  // cout << "depth: " << depth << endl;
  // cout << state << endl;
  ++expanded;
  if (depth == 0 || state.terminal()) return color * state.value();

  int alpha = -numeric_limits<int>::max();
  bool color_b = (color == 1);
  bool no_moves = true;
  for (int pos = 0; pos < DIM; ++pos) {
    if (state.outflank(color_b, pos)) {
      ++generated;
      auto tmp = -negamax(state.move(color_b, pos), depth - 1, -color);
      alpha = max(alpha, tmp);
      // cout << "depth: " << depth << endl;
      // cout << state << endl;
      no_moves = false;
    }
  }
  if (no_moves) {
    ++generated;
    alpha = max(alpha, -negamax(state, depth - 1, -color));
  }
  return alpha;
}

/**
 * Negamax algorithm with alpha-beta pruning
 * @param state current state
 * @param depth current depth
 * @param color current player, 1 for MAX, -1 for MIN
 * @param alpha alpha value
 * @param beta beta value
 * @param use_tt use transposition table
 * @return value of the state
 */
int negamax(state_t state, int depth, int alpha, int beta, int color,
            bool use_tt) {
  // cout << "depth: " << depth << endl;
  // cout << state << endl;
  ++expanded;
  if (depth == 0 || state.terminal()) return color * state.value();

  int value = -numeric_limits<int>::max();
  bool color_b = (color == 1);
  bool no_moves = true;
  for (int pos = 0; pos < DIM; ++pos) {
    if (state.outflank(color_b, pos)) {
      ++generated;
      value = max(value, -negamax(state.move(color_b, pos), depth - 1, -beta,
                                  -alpha, -color));
      // cout << "depth: " << depth << endl;
      // cout << state << endl;
      alpha = max(alpha, value);
      no_moves = false;
      if (alpha >= beta) break;
    }
  }
  if (no_moves) {
    ++generated;
    value = max(value, -negamax(state, depth - 1, -beta, -alpha, -color));
    alpha = max(alpha, value);
  }
  return value;
}

/**
 * Test function for scout algorithm
 * @param state current state
 * @param depth current depth
 * @param score current score
 * @param condition condition to test, can be ">" or ">="
 * @param color current player, 1 for MAX, -1 for MIN
 * @return true if the condition is true, false otherwise
 * @see scout
 */
bool TEST(state_t state, unsigned depth, int score, string condition,
          int color) {
  if (depth == 0 || state.terminal()) {
    if (condition == ">")
      return state.value() > score;
    else if (condition == ">=")
      return state.value() >= score;
  }
  bool is_MAX = (color == 1);
  bool color_b = (color == 1);
  bool no_moves = true;
  for (int pos = 0; pos < DIM; ++pos) {
    if (state.outflank(color_b, pos)) {
      no_moves = false;
      state_t child = state.move(color_b, pos);
      if (is_MAX && TEST(child, depth - 1, score, condition, -color))
        return true;
      else if (!is_MAX && !TEST(child, depth - 1, score, condition, -color))
        return false;
    }
  }
  if (no_moves) {
    if (is_MAX && TEST(state, depth, score, condition, -color))
      return true;
    else if (!is_MAX && !TEST(state, depth, score, condition, -color))
      return false;
  }
  return !is_MAX;
}

/**
 * Scout algorithm
 * @param state current state
 * @param depth current depth
 * @param color current player, 1 for MAX, -1 for MIN
 * @param use_tt use transposition table
 * @return value of the state
 */
int scout(state_t state, int depth, int color, bool use_tt) {
  // cout << "depth: " << depth << endl;
  // cout << state << endl;
  ++expanded;
  if (depth == 0 || state.terminal()) return state.value();

  int score = 0;
  bool is_first_child = true;
  bool color_b = (color == 1);
  bool no_moves = true;
  for (int pos = 0; pos < DIM; ++pos) {
    if (state.outflank(color_b, pos)) {
      state_t child = state.move(color_b, pos);
      ++generated;
      if (is_first_child) {
        score = scout(child, depth - 1, -color);
        is_first_child = false;
      } else {
        if (color == 1 && TEST(child, depth, score, ">", -color))
          score = scout(child, depth - 1, -color);
        if (color == -1 && !TEST(child, depth, score, ">=", -color))
          score = scout(child, depth - 1, -color);
      }
      no_moves = false;
    }
  }
  if (no_moves) {
    ++generated;
    score = scout(state, depth, -color);
  }
  return score;
}

/**
 * Negascout algorithm
 * @param state current state
 * @param depth current depth
 * @param alpha alpha value
 * @param beta beta value
 * @param color current player, 1 for MAX, -1 for MIN
 * @param use_tt use transposition table
 * @return value of the state
 */
int negascout(state_t state, int depth, int alpha, int beta, int color,
              bool use_tt) {
  // cout << state << endl;
  ++expanded;
  if (depth == 0 || state.terminal()) return color * state.value();

  int score = -numeric_limits<int>::max();
  bool color_b = (color == 1);
  bool no_moves = true;
  bool is_first_child = true;
  for (int pos = 0; pos < DIM; ++pos) {
    if (state.outflank(color_b, pos)) {
      state_t child = state.move(color_b, pos);
      ++generated;
      no_moves = false;
      if (is_first_child) {
        score = -negascout(child, depth - 1, -beta, -alpha, -color);
        is_first_child = false;
      } else {
        score = -negascout(child, depth - 1, -alpha - 1, -alpha, -color);
        if (alpha < score && score < beta)
          score = -negascout(child, depth - 1, -beta, -score, -color);
      }
      alpha = max(alpha, score);
      if (alpha >= beta) break;
    }
  }
  if (no_moves) {
    ++generated;
    score = -negascout(state, depth - 1, -beta, -alpha, -color);
    alpha = max(alpha, score);
  }
  return alpha;
}

/**
 * @brief Main function.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 * @return int
 *
 * @details
 * The main function receives the following arguments:
 * - argv[1]: algorithm to use (0: negamax, 1: negamax with alpha-beta pruning,
 * 2: scout, 3: negascout)
 */
int main(int argc, const char **argv) {
  state_t pv[128];
  int npv = 0;
  for (int i = 0; PV[i] != -1; ++i) ++npv;

  int algorithm = 0;
  if (argc > 1) algorithm = atoi(argv[1]);
  bool use_tt = argc > 2;

  // Extract principal variation of the game
  state_t state;
  // cout << state << " " << endl;
  cout << "Extracting principal variation (PV) with " << npv << " plays ... "
       << endl
       << flush;
  for (int i = 0; PV[i] != -1; ++i) {
    bool player = i % 2 == 0;  // black moves first!
    int pos = PV[i];
    pv[npv - i] = state;
    state = state.move(player, pos);
    // cout << state << " " << endl;
  }
  pv[0] = state;
  cout << "done!" << endl;

#if 0
    // print principal variation
    for( int i = 0; i <= npv; ++i )
        cout << pv[npv - i];
#endif

  // Print name of algorithm
  cout << "Algorithm: ";
  if (algorithm == 1)
    cout << "Negamax (minmax version)";
  else if (algorithm == 2)
    cout << "Negamax (alpha-beta version)";
  else if (algorithm == 3)
    cout << "Scout";
  else if (algorithm == 4)
    cout << "Negascout";
  cout << (use_tt ? " w/ transposition table" : "") << endl;

  // Run algorithm along PV (bacwards)
  cout << "Moving along PV:" << endl;
  for (int i = 0; i <= npv; ++i) {
    // cout << pv[i];
    // bool black_moves = i % 2 == 1;
    // cout << endl << "|-------------|" << endl;
    // cout << "| Move: " << (black_moves ? "black" : "white") << " |" << endl;
    // cout << "|-------------|" << endl;
    int value = 0;
    TTable[0].clear();
    TTable[1].clear();
    float start_time = Utils::read_time_in_seconds();
    expanded = 0;
    generated = 0;
    int color = i % 2 == 1 ? 1 : -1;

    try {
      if (algorithm == 1) {
        value = negamax(pv[i], 33, color, use_tt);
      } else if (algorithm == 2) {
        value = negamax(pv[i], 33, -200, 200, color, use_tt);
      } else if (algorithm == 3) {
        value = scout(pv[i], 33, color, use_tt);
      } else if (algorithm == 4) {
        value = negascout(pv[i], 33, -200, 200, color, use_tt);
      }
    } catch (const bad_alloc &e) {
      cout << "size TT[0]: size=" << TTable[0].size()
           << ", #buckets=" << TTable[0].bucket_count() << endl;
      cout << "size TT[1]: size=" << TTable[1].size()
           << ", #buckets=" << TTable[1].bucket_count() << endl;
      use_tt = false;
    }

    float elapsed_time = Utils::read_time_in_seconds() - start_time;

    cout << npv + 1 - i << ". " << (color == 1 ? "Black" : "White")
         << " moves: "
         << "value=" << color * value << ", #expanded=" << expanded
         << ", #generated=" << generated << ", seconds=" << elapsed_time
         << ", #generated/second=" << generated / elapsed_time << endl;
  }

  return 0;
}
