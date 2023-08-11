#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>

using namespace std;

const long long INF = 2e17;

int n_vertices, n_edges;
vector<vector<long long>> adj_list;
vector<vector<long long>> adj_matrix;

struct Cut
{
    set<int> x, y; // x and y are the two disjoint sets of vertices in the cut, such that x U y = V

    long long cut_value()
    {
        long long ret = 0;
        for (int i : x)
            for (int j : y)
                ret += adj_matrix[i][j];
        return ret;
    }
};

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

Cut semi_greedy_maxcut()
{
    // Choose a threshold weight for cutoff
    double alpha = (double)rand() / RAND_MAX;
    long long min_weight = INF, max_weight = -INF;
    for (int i = 0; i < adj_list.size(); i++)
    {
        for (int j = 0; j < adj_list[i].size(); j++)
        {
            min_weight = min(min_weight, adj_list[i][j]);
            max_weight = max(max_weight, adj_list[i][j]);
        }
    }
    long long threshold = alpha * (max_weight - min_weight) + min_weight;

    // Construct Restricted Candidate List
    vector<pair<int, int>> candidates;
    for (int i = 0; i < adj_list.size(); i++)
    {
        for (int j = 0; j < adj_list[i].size(); j++)
        {
            if (i > j)
                continue;
            if (adj_list[i][j] >= threshold)
                candidates.push_back({i, j});
        }
    }

    Cut ret;

    // Choose a random edge from RCL
    set<int> remaining_vertices; // remaining vertices not in X or Y yet
    for (int i = 0; i < n_vertices; i++)
        remaining_vertices.insert(i);
    int idx = rand() % candidates.size();
    pair<int, int> initial = candidates[idx];

    // Add the chosen edge endpoints to X and Y as initial vertices
    ret.x.insert(initial.first);
    ret.y.insert(initial.second);
    remaining_vertices.erase(initial.first);
    remaining_vertices.erase(initial.second);

    while (ret.x.size() + ret.y.size() < n_vertices)
    {
        long long min_x = INF, min_y = INF;   // minimum contribution to cut weight by adding a vertex to X or Y
        long long max_x = -INF, max_y = -INF; // maximum contribution to cut weight by adding a vertex to X or Y
        vector<pair<long long, long long>> cut_values(n_vertices); // cut_values[i] = {cut_x, cut_y} for vertex i
        for (int v : remaining_vertices)
        {
            cut_values[v] = calculate_contribution(v, ret.x, ret.y);
            min_x = min(min_x, cut_values[v].first);
            min_y = min(min_y, cut_values[v].second);
            max_x = max(max_x, cut_values[v].first);
            max_y = max(max_y, cut_values[v].second);
        }

        // Choose a threshold weight for cutoff
        min_weight = min(min_x, min_y);
        max_weight = max(max_x, max_y);
        threshold = alpha * (max_weight - min_weight) + min_weight;

        // Construct Restricted Candidate List
        vector<int> rcl;
        for (int v : remaining_vertices)
        {
            if (cut_values[v].first >= threshold || cut_values[v].second >= threshold)
                rcl.push_back(v);
        }

        // Choose a random vertex from RCL
        idx = rand() % rcl.size();
        int chosen = rcl[idx];

        // Add the chosen vertex to X or Y depending on which cut value is greater
        if (cut_values[chosen].first >= cut_values[chosen].second)
            ret.x.insert(chosen);
        else
            ret.y.insert(chosen);

        // Remove the chosen vertex from remaining set of vertices
        remaining_vertices.erase(chosen);
    }

    return ret;
}

Cut local_search_maxcut(Cut &cut)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < n_vertices && !changed; i++)
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

    bool changed = true;
    Cut c;
    while (changed)
    {
        c = semi_greedy_maxcut();
        c = local_search_maxcut(c);

        long long cut_value = c.cut_value();
        if (cut_value > best_cut_value)
        {
            best_cut_value = cut_value;
            changed = true;
        }
        else
            changed = false;
    }

    return {c, best_cut_value};
}

int main()
{
    srand(time(NULL));
    cin >> n_vertices >> n_edges;
    adj_list.resize(n_vertices);
    adj_matrix.assign(n_vertices, vector<long long>(n_vertices, 0));

    // vertices should be 0-indexed
    for (int i = 0; i < n_edges; i++)
    {
        int u, v, w;
        cin >> u >> v >> w;
        u--;
        v--;
        adj_list[u].push_back(v);
        adj_list[v].push_back(u);
        adj_matrix[u][v] = w;
        adj_matrix[v][u] = w;
    }

    pair<Cut, long long> ans = grasp_maxcut();
    cout << "Cut value: " << ans.second << endl;
    cout << "Vertices in X: ";
    for (int i : ans.first.x)
        cout << i + 1 << " ";
    cout << endl;
    cout << "Vertices in Y: ";
    for (int i : ans.first.y)
        cout << i + 1 << " ";
    cout << endl;
}