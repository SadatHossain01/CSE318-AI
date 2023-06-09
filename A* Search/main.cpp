#include <cassert>
#include <iostream>
#include <ostream>
#include <queue>
#include <set>
#include <stack>
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

    set_parent(par);

    // cout << "This one:\n" << *this << "\n";
    // if (par) cout << "Parent:\n" << *this->parent << "\n\n";

    if (par == nullptr)
      moves = 0;
    else
      moves = par->moves + 1;
  }

  void set_parent(Node *par) {
    if (par)
      this->parent = new Node(*par);
    else
      this->parent = nullptr;
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

  bool operator>(const Node &node) const {
    assert(this->hamming_dist != -1 && node.hamming_dist != -1);
    assert(this->manhattan_dist != -1 && node.manhattan_dist != -1);
    return (this->hamming_dist + this->moves) >
           (node.hamming_dist + node.moves);
  }

  bool operator<(const Node &node) const {
    assert(this->hamming_dist != -1 && node.hamming_dist != -1);
    assert(this->manhattan_dist != -1 && node.manhattan_dist != -1);
    return (this->hamming_dist + this->moves) <
           (node.hamming_dist + node.moves);
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

  Node make_move(int dir) {
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

    Node new_node(new_grid, this);

    return new_node;
  }

  Node &operator=(const Node &node) {
    this->grid = node.grid;
    set_parent(node.parent);
    this->moves = node.moves;
    this->hamming_dist = node.hamming_dist;
    this->manhattan_dist = node.manhattan_dist;
    this->zr = node.zr;
    this->zc = node.zc;
    return *this;
  }

  Node(const Node &node) {
    this->grid = node.grid;
    set_parent(node.parent);
    this->moves = node.moves;
    this->hamming_dist = node.hamming_dist;
    this->manhattan_dist = node.manhattan_dist;
    this->zr = node.zr;
    this->zc = node.zc;
  }

  ~Node() { delete parent; }
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

void A_star(Node &start) {
  priority_queue<Node, vector<Node>, greater<Node>> pq;
  // set<Node> visited;
  pq.push(start);
  int popped = 0;

  bool flag = true;

  while (flag && !pq.empty()) {
    Node cur = pq.top();
    pq.pop();
    popped++;
    // visited.insert(cur);

    // cout << "popped node\n" << cur << "\n";
    // if (cur.parent) cout << "its parent\n" << *cur.parent << "\n";
    // cout << "\n";

    for (int dir = 0; dir < 4; dir++) {
      if (!valid(cur.zr, cur.zc, dir, cur.grid.size(), cur.grid[0].size()))
        continue;
      if (cur.parent != nullptr) {
        if (dir == 0 && cur.parent->zc == cur.zc - 1) continue;
        if (dir == 1 && cur.parent->zc == cur.zc + 1) continue;
        if (dir == 2 && cur.parent->zr == cur.zr - 1) continue;
        if (dir == 3 && cur.parent->zr == cur.zr + 1) continue;
      }

      Node new_node = cur.make_move(dir);

      if (new_node.hamming_dist == 0 && new_node.manhattan_dist == 0) {
        cerr << "Popped Nodes: " << popped << "\n";
        cout << "\nMinimum number of moves = " << new_node.moves << "\n\n";

        stack<Node> st;

        while (new_node.parent != nullptr) {
          // cout << new_node << "\n";
          st.push(new_node);
          new_node = *new_node.parent;
        }

        st.push(new_node);

        while (!st.empty()) {
          cout << st.top() << "\n";
          st.pop();
        }

        flag = false;
        break;
      }

      // cout << "pushed node\n" << new_node << "\n";
      // cout << "its parent\n" << *new_node.parent << "\n\n";
      // if (!visited.count(new_node)) pq.push(new_node);
      pq.push(new_node);
    }
  }
}

int main() {
  int k;
  cin >> k;

  vector<vector<int>> grid(k, vector<int>(k, 0));

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < k; j++) {
      cin >> grid[i][j];
    }
  }

  Node start(grid, nullptr);

  A_star(start);
}