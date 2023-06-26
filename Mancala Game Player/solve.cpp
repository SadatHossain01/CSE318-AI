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
double time_limit_1, time_limit_2;
const double INF = 2e17;
clock_t timer;
map<pair<int, int>, int> m{{{7, 13}, 0},
                           {{14, 19}, 14},
                           {{20, 26}, 13},
                           {{27, 32}, 27},
                           {{33, 39}, 26}};
bool reorder = false;

struct MancalaNode {
 private:
  inline int final_bin(int move_idx, bool turn1) {
    int res = -1;
    int tot = move_idx + p[move_idx];
    if (turn1) {
      res = tot % 13;
    } else {
      for (auto it = m.begin(); it != m.end(); it++) {
        if (it->first.first <= tot && tot <= it->first.second) {
          res = tot - it->second;
          break;
        }
      }
    }
    // cerr << move_idx << " " << p[move_idx] << " " << (turn1 ? "true" :
    // "false")
    //      << " " << res << endl;
    assert(res != -1);
    return res;
  }

  void reorder_moves(vector<int>& moves) {
    set<int> s;
    for (int i = 0; i < moves.size(); i++) {
      s.insert(moves[i]);
    }

    // if any move awards another turn, put it at the front
    // if there are multiple ones, prioritize the one closest to the storage

    vector<int> new_moves;
    const int sz = moves.size();

    for (int i = sz - 1; i >= 0; i--) {
      if (final_bin(moves[i], p1Turn) == 6) {
        new_moves.push_back(moves[i]);
        s.erase(moves[i]);
      }
    }

    // if any move captures stones, prioritize the one that captures the most
    vector<pair<int, int>> temp;
    for (int i = sz - 1; i >= 0; i--) {
      if (s.count(moves[i]) == 0) continue;
      int final = final_bin(moves[i], p1Turn);
      if (p1Turn) {
        if (final < 6 && p[final] == 0 && p[12 - final] > 0) {
          temp.push_back(make_pair(p[12 - final], moves[i]));
          s.erase(moves[i]);
        }
      } else {
        if (final >= 7 && final <= 12 && p[final] == 0 && p[12 - final] > 0) {
          temp.push_back(make_pair(p[12 - final], moves[i]));
          s.erase(moves[i]);
        }
      }
    }

    sort(temp.rbegin(), temp.rend());
    for (int i = 0; i < temp.size(); i++) {
      new_moves.push_back(temp[i].second);
    }

    temp.clear();

    // rest just in ascending order of gems
    for (auto it = s.begin(); it != s.end(); it++) {
      temp.push_back(make_pair(p[*it], *it));
    }

    sort(temp.begin(), temp.end());
    for (int i = 0; i < temp.size(); i++) {
      new_moves.push_back(temp[i].second);
    }

    moves = new_moves;

    // cerr << "Done reordering moves" << endl;
  }

 public:
  vector<int> p;  // 0 to 5 are player 1's pits, 6 is player 1's storage bin,
                  // 7 to 12 are player 2's pits, 13 is player 2's storage bin
  bool p1Turn;    // true if player 1's turn

  MancalaNode() {
    p.assign(14, 4);  // 7th index is storage bin
    p[6] = p[13] = 0;
    p1Turn = true;
  }

  vector<int> get_next_moves() {
    vector<int> moves;
    int start = p1Turn ? 0 : 7;
    for (int i = start; i < start + 6; i++) {
      if (p[i] > 0) moves.push_back(i);
    }

    random_shuffle(moves.begin(), moves.end());
    if (reorder) reorder_moves(moves);

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
    //      << endl;
    assert(idx >= 0 && idx != 6 && idx < 13 && p[idx] > 0);
    assert(p1Turn ? idx < 6 : idx > 6);

    // will return if another move will be awarded to this player

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
        p[last] = p[12 - last] = 0;
      }
    } else {
      if (last >= 7 && last <= 12 && p[last] == 1 && p[12 - last]) {
        p[13] += p[12 - last] + p[last];
        p[last] = p[12 - last] = 0;
      }
    }

    if ((p1Turn && last == 6) || (!p1Turn && last == 13)) {
      // give this player another turn
      return true;
    } else {
      return false;
    }
  }

  double evaluate(int heuristic_idx, int depth) {
    if (heuristic_idx == -1) {
      // game over
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

    // cerr << "Depth: " << depth << endl;
    assert(p1Turn);
    assert(p[6] <= 24 && p[13] <= 24);

    if (heuristic_idx == 3) {
      const double my_store_weight = 1.0, chain_weight = 0.9,
                   opp_store_weight = -0.7, me_close_to_winning = 0.2,
                   opp_close_to_winning = -0.25, my_side_gems_weight = 0.15,
                   number_of_moves_weight = 0.4, capture_weight = 0.5;
      double score = 0;
      int loop_start = 0, loop_end = 6;

      int my_side_gems = accumulate(p.begin(), p.begin() + 6, 0);
      int opp_side_gems = accumulate(p.begin() + 7, p.begin() + 13, 0);

      score += my_side_gems_weight * my_side_gems;
      score += my_store_weight * p[6];
      score += opp_store_weight * p[13];
      score += me_close_to_winning * (1 / (25 - p[6]));
      score += opp_close_to_winning * (1 / (25 - p[13]));

      int non_empty_pits = 0;
      for (int i = loop_start; i < loop_end; i++) {
        if (p[i] > 0) non_empty_pits++;
      }

      score += number_of_moves_weight * non_empty_pits;

      int closest_chain_chance = -1, capture_opportunities = 0;
      for (int i = loop_start; i < loop_end; i++) {
        if (p[i] == 0) continue;
        int final = final_bin(i, true);
        if (final == 6)
          closest_chain_chance =
              max(closest_chain_chance,
                  i);  // the closer to my pit, the better since it might not
                       // disrupt previous pits for subsqeuent chaining
        if (final < 6 && p[final] == 0 && p[12 - final] > 0)
          capture_opportunities = max(capture_opportunities, p[12 - final]);
      }

      if (closest_chain_chance != -1)
        score += chain_weight * sqrt(closest_chain_chance);
      score += capture_weight * capture_opportunities;

      // cerr << "Score: " << score << endl;
      return score;

    } else
      return -1;
  }

  friend ostream& operator<<(ostream& os, const MancalaNode& node) {
    os << "\t\t\tP2\n\nIndex:\t";
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

double minimax(MancalaNode node, int depth, double alpha, double beta,
               bool repeat_move) {
  explored++;

  if (node.is_game_over())
    return node.evaluate(-1, depth);
  else if (node.p[6] > 24 || node.p[13] > 24)
    return node.evaluate(-2, depth);
  if (!repeat_move && node.p1Turn && depth >= 6 &&
      clock() - timer > time_limit_1 * CLOCKS_PER_SEC) {
    // cerr << "TLE at depth " << depth << endl;
    return node.evaluate(3, depth);
  }

  vector<int> moves = node.get_next_moves();

  if (node.p1Turn) {
    double best = -INF;
    int i = 0;

    for (int next_move : moves) {
      i++;
      MancalaNode next_node = node;
      bool another_turn = next_node.execute_move(next_move);

      double score;

      if (another_turn) {
        score = minimax(next_node, depth, alpha, beta, true);
      } else {
        next_node.p1Turn = !next_node.p1Turn;
        score = minimax(next_node, depth + 1, alpha, beta, false);
      }

      // cerr << "Depth " << depth << " Move " << i << " Score " << score <<
      // "\n";

      best = max(best, score);
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
    double best = INF;
    int i = 0;

    for (int next_move : moves) {
      i++;
      MancalaNode next_node = node;
      bool another_turn = next_node.execute_move(next_move);

      double score;

      if (another_turn) {
        score = minimax(next_node, depth, alpha, beta, true);
      } else {
        next_node.p1Turn = !next_node.p1Turn;
        score = minimax(next_node, depth + 1, alpha, beta, false);
      }

      // cerr << "Depth " << depth << " Move " << i << " Score " << score <<
      // "\n";

      best = min(best, score);
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

bool call_my_turn(MancalaNode& node) {
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

bool call_ai_turn(MancalaNode& node) {
  int best_move = -1;
  double best_score = -INF;
  if (!node.p1Turn) {
    best_score = 1000000;
  }
  vector<int> moves = node.get_next_moves();

  cerr << "P" << (node.p1Turn ? 1 : 2) << " moves: ";
  for (int move : moves) {
    cerr << move + 1 << " ";
  }
  cerr << endl;

  time_limit_1 = 4.5 / moves.size();
  for (int next_move : moves) {
    MancalaNode next_node = node;
    bool another_turn = next_node.execute_move(next_move);
    double score;
    // start a timer
    timer = clock();
    if (another_turn) {
      score = minimax(next_node, 0, -INF, INF, false);
    } else {
      score = minimax(next_node, 1, -INF, INF, false);
    }

    cerr << "Move " << next_move + 1 << ", Score: " << score << endl;

    if (node.p1Turn) {
      if (score > best_score) {
        best_score = score;
        best_move = next_move;
      }
    } else {
      if (score < best_score) {
        best_score = score;
        best_move = next_move;
      }
    }
  }
  cerr << "Explored: " << explored << " Pruned: " << pruned << endl;
  cout << "Move for player " << (node.p1Turn ? "1" : "2") << ": "
       << best_move + 1 << endl;
  return node.execute_move(best_move);
}

int main(int argc, char** argv) {
  MancalaNode node;
  cout << node << endl;

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if (string(argv[i]) == "r") reorder = true;
    }
  }

  bool my_turn = false;

  while (!node.is_game_over()) {
    explored = 0;
    pruned = 0;
    bool again = false;
    if (my_turn) {
      again = call_my_turn(node);
    } else {
      again = call_ai_turn(node);
    }
    cout << node << endl << endl;
    if (!again) {
      my_turn = !my_turn;
      node.p1Turn = !node.p1Turn;
    } else {
      cout << "Another turn for player " << (node.p1Turn ? "1" : "2") << endl;
    }
  }

  cout << "Game over!\n";
  pair<int, int> scores = node.compute_final_score();
  cout << "Player 1: " << scores.first << endl;
  cout << "Player 2: " << scores.second << endl;
  if (scores.first > scores.second) {
    cout << "Player 1 wins!\n";
  } else if (scores.first < scores.second) {
    cout << "Player 2 wins!\n";
  } else {
    cout << "It's a tie!\n";
  }

  return 0;
}