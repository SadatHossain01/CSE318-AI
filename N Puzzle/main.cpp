#include <cassert>
#include <iostream>
#include <ostream>
#include <queue>
#include <set>
#include <stack>
#include <unordered_set>
#include <vector>
using namespace std;

struct Node {
 public:
  vector<vector<int>> grid;
  Node *parent = nullptr;
  int moves = 0, hamming_dist = -1, manhattan_dist = -1;
  int zr = 0, zc = 0;

  Node(int k = 0) { grid.assign(k, vector<int>(k, 0)); }

  Node(const vector<vector<int>> &grid, Node *par) {
    this->grid = grid;

    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        if (!grid[i][j]) {
          zr = i;
          zc = j;
          break;
        }
      }
    }

    hamming_dist = this->hamming_distance();
    manhattan_dist = this->manhattan_distance();

    this->parent = par;

    moves = par ? par->moves + 1 : 0;
  }

  int hamming_distance() {
    int dist = 0;

    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        if (grid[i][j] && grid[i][j] != i * grid[i].size() + j + 1) dist++;
      }
    }

    // cout << *this;
    // cout << "hamming distance: " << dist << "\n";

    return dist;
  }

  int manhattan_distance() {
    int dist = 0;
    int actual_row, actual_col;

    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        actual_row = (grid[i][j] - 1) / grid[0].size();
        actual_col = (grid[i][j] - 1) % grid[0].size();
        if (grid[i][j]) dist += abs(i - actual_row) + abs(j - actual_col);
      }
    }

    // cout << *this;
    // cout << "manhattan distance: " << dist << "\n";

    return dist;
  }

  friend ostream &operator<<(ostream &os, const Node &node) {
    for (int i = 0; i < node.grid.size(); i++) {
      for (int j = 0; j < node.grid[i].size(); j++) {
        os << node.grid[i][j];
        os << " ";
      }
      os << "\n";
    }
    return os;
  }

  bool operator==(const Node &node) const {
    assert(grid.size() == node.grid.size() &&
           grid[0].size() == node.grid[0].size());

    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        if (grid[i][j] != node.grid[i][j]) return false;
      }
    }

    return true;
  }

  Node *make_move(int dir) {
    // dir 0, 1, 2, 3 for left, right, up, down respectively

    vector<vector<int>> new_grid = grid;

    if (dir == 0) {
      // left
      assert(zc > 0);
      swap(new_grid[zr][zc], new_grid[zr][zc - 1]);
    } else if (dir == 1) {
      // right
      assert(zc < grid[0].size() - 1);
      swap(new_grid[zr][zc], new_grid[zr][zc + 1]);
    } else if (dir == 2) {
      // up
      assert(zr > 0);
      swap(new_grid[zr][zc], new_grid[zr - 1][zc]);
    } else if (dir == 3) {
      // down
      assert(zr < grid.size() - 1);
      swap(new_grid[zr][zc], new_grid[zr + 1][zc]);
    }

    Node *new_node = new Node(new_grid, this);

    return new_node;
  }
};

bool valid(int zr, int zc, int dir, int ro, int co) {
  // 0, 1, 2, 3 left, right, up, down
  if (dir == 0) {
    // left
    return zc > 0;
  } else if (dir == 1) {
    // right
    return zc < co - 1;
  } else if (dir == 2) {
    // up
    return zr > 0;
  } else if (dir == 3) {
    // down
    return zr < ro - 1;
  }
  return false;
}

bool solvable(const vector<vector<int>> &grid, int zr) {
  int inversions = 0;

  vector<int> temp;
  for (int i = 0; i < grid.size(); i++) {
    temp.insert(temp.end(), grid[i].begin(), grid[i].end());
  }

  for (int i = 0; i < temp.size(); i++) {
    for (int j = i + 1; j < temp.size(); j++) {
      if (temp[i] && temp[j] && temp[i] > temp[j]) inversions++;
    }
  }

  if (grid.size() % 2) {
    // if odd, then number of inversions must be even
    return inversions % 2 == 0;
  }

  else {
    int diff = grid.size() - zr;
    return diff % 2 != inversions % 2;
  }
}

class Compare {
 public:
  int type;  // 0 for hamming, 1 for manhattan
  Compare(int type = 1) : type(type) {}
  bool operator()(Node *a, Node *b) const {
    // since we want smallest first, we reverse the comparison (we overload >
    // instead of <)
    if (!type) {
      assert(a->hamming_dist != -1 && b->hamming_dist != -1);
      return a->hamming_dist + a->moves > b->hamming_dist + b->moves;
    } else {
      assert(a->manhattan_dist != -1 && b->manhattan_dist != -1);
      return a->manhattan_dist + a->moves > b->manhattan_dist + b->moves;
    }
  }
};

void A_star(Node *start, bool manhattan) {
  Compare comp(manhattan);
  priority_queue<Node *, vector<Node *>, Compare> pq(comp);
  set<vector<vector<int>>> visited;
  vector<Node *> pointer_to_be_deleted;
  int expanded = 0;

  pq.push(start);
  int explored = 1;
  bool flag = true;

  while (flag) {
    Node *cur = pq.top();
    pq.pop();
    expanded++;
    visited.insert(cur->grid);
    pointer_to_be_deleted.push_back(cur);

    for (int dir = 0; dir < 4; dir++) {
      if (!valid(cur->zr, cur->zc, dir, cur->grid.size(), cur->grid[0].size()))
        continue;

      Node *new_node = cur->make_move(dir);
      if (visited.count(new_node->grid)) {
        delete new_node;
        continue;
      }

      if (new_node->hamming_dist == 0 && new_node->manhattan_dist == 0) {
        cout << "\nNumber of nodes explored = " << explored << "\n";
        cout << "Number of nodes expanded = " << expanded << "\n";
        cout << "Minimum number of moves = " << new_node->moves << "\n\n";

        stack<Node *> st;
        Node *last = new_node;

        while (new_node->parent != nullptr) {
          st.push(new_node);
          new_node = new_node->parent;
        }

        st.push(new_node);

        while (!st.empty()) {
          cout << *(st.top()) << "\n";
          st.pop();
        }

        delete last;

        flag = false;
        break;
      }

      else {
        pq.push(new_node);
        explored++;
      }
    }
  }

  while (!pq.empty()) {
    delete pq.top();
    pq.pop();
  }
  for (auto it : pointer_to_be_deleted) {
    delete it;
  }
}

int main() {
  int k;
  cin >> k;

  vector<vector<int>> grid(k, vector<int>(k, 0));
  int zr = 0;

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < k; j++) {
      cin >> grid[i][j];
      if (grid[i][j] == 0) zr = i;
    }
  }

  if (!solvable(grid, zr)) {
    cout << "Not solvable\n";
    return 0;
  }

  Node *start = new Node(grid, nullptr);
  cout << "Using Manhattan distance\n";
  A_star(start, true);

  //   start = new Node(grid, nullptr);
  //   cout << "\n\nUsing Hamming distance\n";
  //   A_star(start, false);
}