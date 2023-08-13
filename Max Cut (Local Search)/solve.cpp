#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>

using namespace std;

#ifdef LOCAL
#include "debug.h"
#else
#define debug(...)
#endif

struct Edge
{
    int u, v;
    long long w;
};

struct Cut
{
    set<int> x, y; // x and y are the two disjoint sets of vertices in the cut, such that x U y = V

    long long cut_value(const vector<vector<long long>> &adj_matrix)
    {
        long long ret = 0;
        for (int i : x)
            for (int j : y)
                ret += adj_matrix[i][j];
        return ret;
    }
};

const long long INF = 2e17;
const int MAX_ITER = 500;
const int CUTOFF_SECONDS = 60;
double alpha;
double alpha_1_cutoff = 0.999995; // if alpha > alpha_1_cutoff, assume alpha to be 1 and use greedy choice
int n_vertices, n_edges;
vector<vector<long long>> adj_matrix;
long long min_weight = INF, max_weight = -INF;
Edge best_edge = {-1, -1, -INF}, worst_edge = {-1, -1, INF};
enum SOLUTION_TYPE
{
    SEMI_GREEDY,
    RANDOMIZED,
} solution_type;

pair<long long, long long> calculate_contribution(int v, const set<int> &x, const set<int> &y)
{
    // cut_value(v) = sum of weights of edges between v and vertices in the other set
    // cut_x will denote the incremental contribution to the cut weight resulting from adding v to set X
    // cut_y will denote the incremental contribution to the cut weight resulting from adding v to set Y
    long long cut_x = 0, cut_y = 0;
    for (int node : y)
        cut_x += adj_matrix[v][node];
    for (int node : x)
        cut_y += adj_matrix[v][node];
    return {cut_x, cut_y};
}

int choose_next_candidate(const set<int> &rem, const vector<pair<long long, long long>> &cut_values,
                          long long threshold)
{
    // Construct Restricted Candidate List
    vector<int> rcl;
    for (int v : rem)
        if (max(cut_values[v].first, cut_values[v].second) >= threshold)
            rcl.push_back(v);

    // Choose a random vertex from RCL
    int idx = rand() % rcl.size();
    return rcl[idx];
}

Cut randomized_maxcut()
{
    Cut ret;
    for (int i = 1; i <= n_vertices; i++)
        if (rand() % 2)
            ret.x.insert(i);
        else
            ret.y.insert(i);
    return ret;
}

Cut semi_greedy_maxcut()
{
    Cut ret;
    set<int> remaining_vertices; // remaining vertices not in X or Y yet
    for (int i = 1; i <= n_vertices; i++)
        remaining_vertices.insert(i);

    Edge initial;

    if (alpha > alpha_1_cutoff)
        initial = best_edge; // greedy choice, use the best edge
    else
    {
        // Choose a threshold weight for cutoff
        long long threshold = alpha * (max_weight - min_weight) + min_weight;

        // Construct Restricted Candidate List
        vector<pair<int, int>> candidates;
        for (int i = 1; i <= n_vertices; i++)
            for (int j = i + 1; j <= n_vertices; j++)
                if (adj_matrix[i][j] >= threshold)
                    candidates.push_back({i, j});

        // Choose a random edge from RCL
        int idx = rand() % candidates.size();

        initial = {candidates[idx].first, candidates[idx].second,
                   adj_matrix[candidates[idx].first][candidates[idx].second]};
    }

    // Add the chosen vertices to X and Y as initial vertices
    ret.x.insert(initial.u);
    ret.y.insert(initial.v);
    remaining_vertices.erase(initial.u);
    remaining_vertices.erase(initial.v);

    while (ret.x.size() + ret.y.size() < n_vertices)
    {
        long long min_x = INF, min_y = INF;   // minimum contribution to cut weight by adding a vertex to X or Y
        long long max_x = -INF, max_y = -INF; // maximum contribution to cut weight by adding a vertex to X or Y
        vector<pair<long long, long long>> cut_values(n_vertices + 1); // cut_values[i] = {cut_x, cut_y} for vertex i

        pair<int, long long> max_vertex = {-1, -INF};
        for (int v : remaining_vertices)
        {
            cut_values[v] = calculate_contribution(v, ret.x, ret.y);
            if (max(cut_values[v].first, cut_values[v].second) > max_vertex.second)
                max_vertex = {v, max(cut_values[v].first, cut_values[v].second)};
            min_x = min(min_x, cut_values[v].first);
            min_y = min(min_y, cut_values[v].second);
            max_x = max(max_x, cut_values[v].first);
            max_y = max(max_y, cut_values[v].second);
        }

        // Choose a vertex based on alpha and the cut values
        min_weight = min(min_x, min_y);
        max_weight = max(max_x, max_y);
        int chosen;
        if (alpha > alpha_1_cutoff)
            chosen = max_vertex.first; // greedy choice, use the best vertex
        else
            chosen =
                choose_next_candidate(remaining_vertices, cut_values, alpha * (max_weight - min_weight) + min_weight);

        // Add the chosen vertex to X or Y depending on which cut value is greater
        if (cut_values[chosen].first >= cut_values[chosen].second)
            ret.x.insert(chosen);
        else
            ret.y.insert(chosen);

        // Remove the chosen vertex from remaining set of vertices
        remaining_vertices.erase(chosen);
    }
    assert(remaining_vertices.empty());
    return ret;
}

Cut local_search_maxcut(Cut &cut)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 1; i <= n_vertices && !changed; i++)
        {
            pair<long long, long long> contribution = calculate_contribution(i, cut.x, cut.y);
            bool present_in_x = find(cut.x.begin(), cut.x.end(), i) != cut.x.end();

            if (present_in_x && contribution.second > contribution.first)
            {
                // better to move vertex i from X to Y
                cut.x.erase(i);
                cut.y.insert(i);
                changed = true;
            }
            else if (!present_in_x && contribution.first > contribution.second)
            {
                // better to move vertex i from Y to X
                cut.y.erase(i);
                cut.x.insert(i);
                changed = true;
            }
        }
    }
    return cut;
}

pair<Cut, long long> grasp_maxcut()
{
    long long best_cut_value = -INF;
    clock_t start = clock();
    Cut c;
    int iter = 0;

    auto should_continue = [&]() -> bool {
        if (alpha > alpha_1_cutoff)
            return iter < 1; // greedy, no need to run again and again
        else
            return ((clock() - start) / CLOCKS_PER_SEC) < CUTOFF_SECONDS;
    };

    while (should_continue())
    {
        max_weight = best_edge.w;
        min_weight = worst_edge.w;
        if (solution_type == SEMI_GREEDY)
            c = semi_greedy_maxcut();
        else if (solution_type == RANDOMIZED)
            c = randomized_maxcut();
        else
            assert(false);
        c = local_search_maxcut(c);

        long long cut_value = c.cut_value(adj_matrix);
        if (cut_value > best_cut_value)
            best_cut_value = cut_value;
        cout << "Iteration " << ++iter << ":\t" << cut_value << endl;
    }

    return {c, best_cut_value};
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    solution_type = SEMI_GREEDY;
    alpha = (double)rand() / RAND_MAX;

    if (argc >= 2)
    {
        if (string(argv[1]) == "g")
            alpha = 1; // greedy is basically semi-greedy with alpha = 1
        else if (string(argv[1]) == "r")
            solution_type = RANDOMIZED;
    }

    if (solution_type == SEMI_GREEDY)
        cout << "Alpha: " << alpha << endl;

    cin >> n_vertices >> n_edges;
    adj_matrix.assign(n_vertices + 1, vector<long long>(n_vertices + 1, 0));

    // input vertices are 1-indexed
    for (int i = 0; i < n_edges; i++)
    {
        int u, v;
        long long w;
        cin >> u >> v >> w;
        adj_matrix[u][v] += w;
        adj_matrix[v][u] += w;
        if (w > best_edge.w)
            best_edge = {u, v, w};
        if (w < worst_edge.w)
            worst_edge = {u, v, w};
    }

    pair<Cut, long long> ans = grasp_maxcut();

    cout << "Cut value: " << ans.second << endl;
}