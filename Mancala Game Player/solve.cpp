#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <tuple>
#include <vector>
using namespace std;

int explored = 0, pruned = 0;
const double INF = 2e17;
int MAX_DEPTH;
clock_t timer;
double time_limit = 0.9;  // for each branch
enum GameMode { HUMAN_AI, AI_AI };
GameMode cur_mode;

struct MancalaNode {
 public:
  vector<int> p;  // 0 to 5 are player 1's pits, 6 is player 1's storage bin,
                  // 7 to 12 are player 2's pits, 13 is player 2's storage bin
  bool p1Turn;    // true if player 1's turn
  int captured_gems, moves_won;

  MancalaNode() {
    p.assign(14, 4);  // 7th index is storage bin
    p[6] = p[13] = 0;
    captured_gems = moves_won = 0;
    p1Turn = true;
  }

  vector<int> get_next_moves() {
    vector<int> moves;
    int start = p1Turn ? 0 : 7;
    for (int i = start; i < start + 6; i++) {
      if (p[i] > 0) moves.push_back(i);
    }

    random_shuffle(moves.begin(), moves.end());

    return moves;
  }

  bool is_game_over() {
    bool p1Empty = true;
    bool p2Empty = true;
    for (int i = 0; i < 6; i++) {
      if (p[i] > 0) p1Empty = false;
      if (p[i + 7] > 0) p2Empty = false;
    }
    return p1Empty || p2Empty;
  }

  pair<int, int> compute_final_score() {
    int p1_score = accumulate(p.begin(), p.begin() + 6, 0);
    int p2_score = accumulate(p.begin() + 7, p.begin() + 13, 0);
    p1_score += p[6];
    p2_score += p[13];
    return make_pair(p1_score, p2_score);
  }

  bool execute_move(int idx) {
    // cerr << "Move for " << (p1Turn ? "player 1" : "player 2") << ": " << idx
    // << endl;
    assert(idx >= 0 && idx != 6 && idx < 13 && p[idx] > 0);
    assert(p1Turn ? idx < 6 : idx > 6);

    // will return if a move is repeated

    int stones = p[idx];
    p[idx] = 0;
    int i = idx + 1;
    while (stones > 0) {
      if (p1Turn && i == 13) {
        i = 0;
        continue;
      }
      if (!p1Turn && i == 6) {
        i++;
        continue;
      }
      p[i]++;
      stones--;
      i++;
      if (i == 14) i = 0;
    }

    // check if last stone was placed in empty pit
    int last = i - 1;
    if (last == -1) last = 13;

    if (p1Turn) {
      if (last <= 5 && p[last] == 1 && p[12 - last]) {
        p[6] += p[12 - last] + p[last];
        captured_gems += p[12 - last] + p[last];
        // cerr << "P1 captured " << p[12 - last] + p[last] << " gems" << endl;
        p[last] = p[12 - last] = 0;
      }
    } else {
      if (last >= 7 && last <= 12 && p[last] == 1 && p[12 - last]) {
        p[13] += p[12 - last] + p[last];
        captured_gems -= p[12 - last] + p[last];
        // cerr << "P2 captured " << p[12 - last] + p[last] << " gems" << endl;
        p[last] = p[12 - last] = 0;
      }
    }

    if ((p1Turn && last == 6) || (!p1Turn && last == 13)) {
      // give this player another turn
      moves_won += (p1Turn ? 1 : -1);
      return true;
    } else {
      return false;
    }
  }

  double evaluate(int heuristic_idx, int depth) {
    if (heuristic_idx == -1) {
      // game literally over
      int p1_score, p2_score;
      tie(p1_score, p2_score) = compute_final_score();
      if (p1_score == p2_score)
        return 0;
      else
        return 1000000000LL * (p1_score - p2_score);
    } else if (heuristic_idx == -2) {
      // not over on paper, but effectively over
      if (p[6] > 24) return 1000000000LL * (2 * p[6] - 48);
      if (p[13] > 24) return -1000000000LL * (2 * p[13] - 48);
    }

    // the game is not yet over
    // cerr << "Depth: " << depth << endl;

    int my_side_gems = accumulate(p.begin(), p.begin() + 6, 0);
    int opp_side_gems = accumulate(p.begin() + 7, p.begin() + 13, 0);
    int my_store = p[6], opp_store = p[13];

    double score = 0;

    if (heuristic_idx == 1) {
      score = my_store - opp_store;
    } else if (heuristic_idx == 2) {
      score =
          0.75 * (my_store - opp_store) + 0.25 * (my_side_gems - opp_side_gems);
    } else if (heuristic_idx == 3) {
      score = 0.7 * (my_store - opp_store) +
              0.2 * (my_side_gems - opp_side_gems) + 0.5 * moves_won;
    } else if (heuristic_idx == 4) {
      score = 1 * (my_store - opp_store) +
              0.55 * (my_side_gems - opp_side_gems) + 4 * moves_won +
              2 * captured_gems;
    } else if (heuristic_idx == 5) {
      const double store_weight = 1.2, side_gems_weight = 1,
                   moves_won_weight = 3, capture_weight = 1.5,
                   best_cap_weight = 1.75;

      score += store_weight * (my_store - opp_store);
      score += side_gems_weight * (my_side_gems - opp_side_gems);
      score += moves_won_weight * moves_won;
      score += capture_weight * captured_gems;

      int best_cap_p1 = 0, best_cap_p2 = 0;
      for (int i = 0; i < 6; i++) {
        if (p[i] <= 1) best_cap_p1 = max(best_cap_p1, p[i + 7]);
        if (p[i + 7] <= 1) best_cap_p2 = max(best_cap_p2, p[i]);
      }
      score += best_cap_weight * (best_cap_p1 - best_cap_p2);
    }

    return score;
  }

  friend ostream& operator<<(ostream& os, const MancalaNode& node) {
    os << "\n\t\t\tP2\n\nIndex:\t";
    // show the indices over the pits
    for (int i = 12; i >= 7; i--) {
      os << i + 1 << "\t";
    }
    os << "\n\t";
    // underline the indices over the pits
    for (int i = 12; i >= 7; i--) {
      os << "-\t";
    }
    os << "\nGems:\t";
    for (int i = 12; i >= 7; i--) {
      os << node.p[i] << "\t";
    }
    os << "\n\n"
       << node.p[13] << "\t\t\t\t\t\t\t" << node.p[6] << "\n\nGems:\t";
    for (int i = 0; i < 6; i++) {
      os << node.p[i] << "\t";
    }
    os << "\n\t";
    for (int i = 0; i < 6; i++) {
      os << "-\t";
    }
    os << "\nIndex:\t";
    for (int i = 0; i < 6; i++) {
      os << i + 1 << "\t";
    }
    os << "\n\n\t\t\tP1\n";
    return os;
  }
};

int best_move = -1;
clock_t start_timer;

double minimax(MancalaNode node, int depth, double alpha, double beta,
               bool repeat_move, int heuristics_index) {
  explored++;
  vector<int> moves = node.get_next_moves();
  if (depth == 0) {
    assert(moves.size() > 0);
    best_move = moves[0];
  }

  if (node.is_game_over())
    return node.evaluate(-1, depth);
  else if (node.p[6] > 24 || node.p[13] > 24)
    return node.evaluate(-2, depth);
  else if (!repeat_move && depth >= MAX_DEPTH &&
           (cur_mode == AI_AI ||
            clock() - start_timer >= time_limit * CLOCKS_PER_SEC)) {
    return node.evaluate(heuristics_index, depth);
  }

  double best, score;

  if (node.p1Turn) {
    best = -INF;
    int i = 0;

    for (int next_move : moves) {
      i++;
      MancalaNode next_node = node;

      if (depth == 0) start_timer = clock();

      bool another_turn = next_node.execute_move(next_move);
      if (!another_turn) next_node.p1Turn = !next_node.p1Turn;
      score = minimax(next_node, depth + 1, alpha, beta, another_turn,
                      heuristics_index);

      // cerr << "Depth " << depth << " Move " << i << " Score " << score <<
      // "\n";

      if (score > best) {
        best = score;
        if (depth == 0) best_move = next_move;
      }
      alpha = max(alpha, best);

      if (alpha >= beta) {
        // cerr << "Pruned " << moves.size() - i << " moves\n";
        pruned += moves.size() - i;
        break;
      }
    }

    return best;
  }

  else {
    best = INF;
    int i = 0;

    for (int next_move : moves) {
      i++;
      MancalaNode next_node = node;

      if (depth == 0) start_timer = clock();

      bool another_turn = next_node.execute_move(next_move);
      if (!another_turn) next_node.p1Turn = !next_node.p1Turn;
      score = minimax(next_node, depth + 1, alpha, beta, another_turn,
                      heuristics_index);

      // cerr << "Depth " << depth << " Move " << i << " Score " << score <<
      // "\n";

      if (score < best) {
        best = score;
        if (depth == 0) best_move = next_move;
      }
      beta = min(beta, best);

      if (alpha >= beta) {
        // cerr << "Pruned " << moves.size() - i << " moves\n";
        pruned += moves.size() - i;
        break;
      }
    }

    return best;
  }
}

bool call_human_turn(MancalaNode& node) {
  int move = -1;
  cout << "Your Move: (" << (node.p1Turn ? "1 to 6" : "8 to 13") << "): ";

  while (cin >> move) {
    move--;
    if (node.p1Turn && move >= 0 && move <= 5 && node.p[move] > 0) break;
    if (!node.p1Turn && move >= 7 && move <= 12 && node.p[move] > 0) break;
    cout << "Invalid move. Try again: ";
  }

  cout << "Move for player " << (node.p1Turn ? "1" : "2") << ": " << move + 1
       << endl;
  return node.execute_move(move);
}

bool call_ai_turn(MancalaNode& node, int heuristics_index) {
  best_move = -1;
  timer = clock();
  double score = minimax(node, 0, -INF, INF, false, heuristics_index);
  cerr << "Time taken: " << (double)(clock() - timer) / CLOCKS_PER_SEC << "s\n";
  assert(best_move != -1);
  cerr << "Explored: " << explored << " Pruned: " << pruned << endl;
  cout << "Move for AI: " << best_move + 1 << endl;
  return node.execute_move(best_move);
}

void show_results(MancalaNode& node, bool human_p1, int h1, int h2) {
  cout << "Game over!\n";
  pair<int, int> scores = node.compute_final_score();
  string name1, name2;

  if (cur_mode == HUMAN_AI) {
    name1 = "You";
    name2 = "AI";
    if (!human_p1) swap(name1, name2);
  } else if (cur_mode == AI_AI) {
    name1 = "AI P1 (Heuristic " + to_string(h1) + ")";
    name2 = "AI P2 (Heuristic " + to_string(h2) + ")";
  }

  cout << name1 << "\t: " << scores.first << endl;
  cout << name2 << "\t: " << scores.second << endl;

  if (scores.first > scores.second) {
    cout << name1 << " win" << (name1 == "You" ? "" : "s") << "!\n";
  } else if (scores.first < scores.second) {
    cout << name2 << " win" << (name2 == "You" ? "" : "s") << "!\n";
  } else {
    cout << "It's a tie!\n";
  }
}

void run_game(bool& my_turn, int h1, int h2) {
  MancalaNode node;
  cout << node << endl;
  bool human_p1 = my_turn;

  while (!node.is_game_over()) {
    explored = pruned = 0;
    bool again = false;

    if (cur_mode == HUMAN_AI) {
      if (my_turn) {
        again = call_human_turn(node);
      } else {
        again = call_ai_turn(node, h1);  // AI will play here with h1 heuristic
      }
    } else if (cur_mode == AI_AI) {
      if (node.p1Turn) {
        again = call_ai_turn(node, h1);
      } else {
        again = call_ai_turn(node, h2);
      }
    }

    cout << node << endl << endl;
    if (!again) {
      if (cur_mode == HUMAN_AI) my_turn = !my_turn;
      node.p1Turn = !node.p1Turn;
    } else {
      cout << "Another turn for player " << (node.p1Turn ? "1" : "2") << endl;
    }
  }

  show_results(node, human_p1, h1, h2);
}

void ai_ai() {
  cur_mode = AI_AI;
  int h1 = 0, h2 = 0;
  while (h1 < 1 || h1 > 5) {
    cout << "Enter AI Player 1 Heuristic (1-5): ";
    cin >> h1;
  }
  while (h2 < 1 || h2 > 5) {
    cout << "Enter AI Player 2 Heuristic (1-5): ";
    cin >> h2;
  }

  bool any;
  MAX_DEPTH = 10;
  run_game(any, h1, h2);
}

void human_ai() {
  cur_mode = HUMAN_AI;
  int p = 0;
  while (p < 1 || p > 2) {
    cout << "Which player do you want to play as? (1/2): ";
    cin >> p;
    cout << endl;
  }

  bool my_turn = (p == 1);
  MAX_DEPTH = 12;
  run_game(my_turn, 5, 5);
}

int main() {
  cout << "Choose the game mode:\n";
  cout << "1. Human vs AI\n";
  cout << "2. AI vs AI\n\n";

  int mode = 0;
  while (mode < 1 || mode > 2) cin >> mode;

  if (mode == 1) {
    human_ai();
  } else {
    ai_ai();
  }

  return 0;
}