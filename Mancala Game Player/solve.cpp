#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

int explored = 0;
double time_limit_1, time_limit_2;
const long long INF = 2e17;
clock_t timer;

struct MancalaNode {
 public:
  vector<int> p;  // 0 to 5 are player 1's pits, 6 is player 1's storage bin,
                  // 7 to 12 are player 2's pits, 13 is player 2's storage bin
  bool p1Turn;    // true if player 1's turn

  MancalaNode() {
    p.assign(14, 4);  // 7th index is storage bin
    p[6] = 0;
    p[13] = 0;
    p1Turn = true;
  }

  vector<int> get_next_moves() {
    vector<int> moves;
    int start = p1Turn ? 0 : 7;
    for (int i = start; i < start + 6; i++) {
      if (p[i] > 0) moves.push_back(i);
    }
    // cerr << "Possible moves for " << (p1Turn ? "player 1" : "player 2") << ":
    // "; for (int move : moves) {
    //   cerr << move << " ";
    // }
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

  bool execute_move(int idx) {
    // cerr << "Move for " << (p1Turn ? "player 1" : "player 2") << ": " << idx
    //      << endl;
    assert(idx >= 0 && idx < 14 && p[idx] > 0);
    assert(p1Turn ? idx < 6 : (idx > 6 && idx < 13));

    // will return if another move will be awarded to this player

    int stones = p[idx];
    p[idx] = 0;
    int i = idx + 1;
    while (stones > 0) {
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

  long long evaluate(int heuristic_idx) {
    if (is_game_over()) {
      int p1_score = accumulate(p.begin(), p.begin() + 6, 0);
      int p2_score = accumulate(p.begin() + 7, p.begin() + 13, 0);
      if (p1_score == 0) {
        return -1000000000LL * (48 - 2 * p[6]);
      } else if (p2_score == 0) {
        return 1000000000LL * (48 - 2 * p[13]);
      } else
        return 0;
    }

    // game effectively over
    if (p[6] > 24) return 1000000000LL * (2 * p[6] - 48);
    if (p[13] > 24) return -1000000000LL * (2 * p[13] - 48);

    if (heuristic_idx == 1) {
      // stones in my storage - stones in opponent's storage
      return p[6] - p[13];
    } else if (heuristic_idx == 2) {
      // W1 * (stones_in_my_storage – stones_in_opponents_storage) +
      // W2 * (stones_on_my_side – stones_on_opponents_side)
      int stones_in_my_storage = p[6];
      int stones_in_opponents_storage = p[13];
      int stones_on_my_side = 0;
      int stones_on_opponents_side = 0;
      for (int i = 0; i < 6; i++) {
        stones_on_my_side += p[i];
        stones_on_opponents_side += p[i + 7];
      }
      return 2 * (stones_in_my_storage - stones_in_opponents_storage) +
             (stones_on_my_side - stones_on_opponents_side);
    } else if (heuristic_idx == 3) {
      long long score = 0;
      score += 45 * (p[6] - p[13]);  // difference between mancalas

      // count capturing opportunities
      int cap1 = 0, cap2 = 0;
      for (int i = 0; i < 6; i++) {
        if (p[i] && i + p[i] < 6 && p[i + p[i]] == 0 && p[12 - i - p[i]])
          cap1 += p[12 - i - p[i]] + 1;
      }
      for (int i = 7; i < 13; i++) {
        if (p[i] && i + p[i] < 13 && p[i + p[i]] == 0 && p[12 - i - p[i]])
          cap2 += p[12 - i - p[i]] + 1;
      }
      if (p1Turn) {
        score += 10 * cap1;
        score -= 20 * cap2;
      } else {
        score += 10 * cap2;
        score -= 20 * cap1;
      }

      // count chaining opportunities
      int chain1 = 0, chain2 = 0;
      for (int i = 0; i < 6; i++) {
        if (p[i] && i + p[i] == 6) chain1++;
      }
      for (int i = 7; i < 13; i++) {
        if (p[i] && i + p[i] == 13) chain2++;
      }
      score += 10 * chain1 - 20 * chain2;

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

long long minimax(MancalaNode node, int depth, long long alpha, long long beta,
                  bool first_call) {
  explored++;
  // if (explored % 100000 == 0) cerr << "Explored: " << explored << endl;
  // cerr << "Depth: " << depth
  //      << " Turn: " << (node.p1Turn ? "player 1" : "player 2") << endl;

  if (node.is_game_over() ||
      clock() - timer > time_limit_2 * CLOCKS_PER_SEC && depth >= 7) {
    return node.evaluate(3);
  }

  if (node.p1Turn) {
    vector<int> moves = node.get_next_moves();
    long long best = -INF;
    int i = 0;

    for (int next_move : moves) {
      i++;
      MancalaNode next_node = node;
      bool another_turn = next_node.execute_move(next_move);
      if (first_call) {
        timer = clock();
        time_limit_2 = time_limit_1 / moves.size();
      }
      if (another_turn) {
        best = max(best, minimax(next_node, depth, alpha, beta, false));
      } else {
        next_node.p1Turn = !next_node.p1Turn;
        best = max(best, minimax(next_node, depth + 1, alpha, beta, false));
      }

      alpha = max(alpha, best);
      if (alpha >= beta) {
        // cerr << "Pruned " << moves.size() - i << " moves\n";
        break;
      }
    }

    return best;
  }

  else {
    vector<int> moves = node.get_next_moves();
    long long best = INF;
    int i = 0;

    for (int next_move : moves) {
      i++;
      MancalaNode next_node = node;
      bool another_turn = next_node.execute_move(next_move);

      if (another_turn) {
        best = min(best, minimax(next_node, depth, alpha, beta, false));
      } else {
        next_node.p1Turn = !next_node.p1Turn;
        best = min(best, minimax(next_node, depth + 1, alpha, beta, false));
      }

      beta = min(beta, best);
      if (alpha >= beta) {
        // cerr << "Pruned " << moves.size() - i << " moves\n";
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
  long long best_score = -INF;
  if (!node.p1Turn) {
    best_score = 1000000;
  }
  vector<int> moves = node.get_next_moves();
  // cerr << "Moves: ";
  // for (int i : moves) cerr << i << " ";
  // cerr << endl;
  time_limit_1 = 4.5 / moves.size();
  for (int next_move : moves) {
    MancalaNode next_node = node;
    bool another_turn = next_node.execute_move(next_move);
    long long score;
    // start a timer
    if (another_turn) {
      score = minimax(next_node, 0, -INF, INF, true);
    } else {
      score = minimax(next_node, 1, -INF, INF, true);
    }
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
  cerr << "Explored: " << explored << endl;
  cout << "Move for player " << (node.p1Turn ? "1" : "2") << ": "
       << best_move + 1 << endl;
  return node.execute_move(best_move);
}

int main() {
  MancalaNode node;
  cout << node << endl;

  bool my_turn = false;

  while (!node.is_game_over()) {
    explored = 0;
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

  return 0;
}