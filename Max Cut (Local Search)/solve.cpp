#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>

#ifdef LOCAL
#include "debug.h"
#else
#define debug(...)
#endif

enum SOLUTION_TYPE
{
    GREEDY_1,
    SEMI_GREEDY_1,
    RANDOMIZED
};

inline std::string enum_to_string(SOLUTION_TYPE st)
{
    switch (st)
    {
    case GREEDY_1:
        return "Greedy-1";
    case SEMI_GREEDY_1:
        return "Semi-Greedy-1";
    case RANDOMIZED:
        return "Randomized";
    default:
        return "none";
    }
}

struct Edge
{
    int u, v;
    long long w;
};

struct Cut
{
    std::set<int> x, y; // x and y are the two disjoint sets of vertices in the cut, such that x U y = V

    long long cut_value(const std::vector<std::vector<long long>> &adj_matrix)
    {
        long long ret = 0;
        for (int i : x)
            for (int j : y)
                ret += adj_matrix[i][j];
        return ret;
    }
};

struct Result
{
    std::string file_name;
    int n_vertices, n_edges;
    SOLUTION_TYPE construction_type;
    long long construction_cut_value, local_search_cut_value, GRASP_cut_value;
    int local_iterations, GRASP_iterations;

    Result(const std::string &file, int n_v, int n_e, SOLUTION_TYPE type)
    {
        file_name = file;
        n_vertices = n_v, n_edges = n_e;
        construction_type = type;
        construction_cut_value = local_search_cut_value = -1;
        local_iterations = GRASP_iterations = -1;
    }

    friend std::ostream &operator<<(std::ostream &os, const Result &res)
    {
        os << res.file_name << "\t|V| = " << res.n_vertices << "\t|E| = " << res.n_edges << "\t"
           << enum_to_string(res.construction_type) << " Cut Value = " << res.construction_cut_value
           << "\t\tLocal Cut Value = " << res.local_search_cut_value
           << "\tLocal Search Iterations = " << res.local_iterations;
        if (res.construction_type == SEMI_GREEDY_1)
            os << "\tGRASP Iterations = " << res.GRASP_iterations << "\tGRASP Cut Value = " << res.GRASP_cut_value;
        return os;
    }
};

const long long INF = 2e17;
const int MIN_ITER = 60, MAX_ITER = 10000;
double alpha;
const double EPS = 1e-8;
int n_vertices, n_edges;
std::vector<std::vector<long long>> adj_matrix;
Edge best_edge = {-1, -1, -INF}, worst_edge = {-1, -1, INF};

std::pair<long long, long long> calculate_contribution(int v, const std::set<int> &x, const std::set<int> &y)
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

int choose_next_candidate(const std::set<int> &remaining_vertices,
                          const std::vector<std::pair<long long, long long>> &cut_values, long long threshold)
{
    // Construct Restricted Candidate List
    std::vector<int> rcl;
    for (int v : remaining_vertices)
        if (std::max(cut_values[v].first, cut_values[v].second) >= threshold)
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
    std::set<int> remaining_vertices; // remaining vertices not in X or Y yet
    for (int i = 1; i <= n_vertices; i++)
        remaining_vertices.insert(i);

    Edge initial;

    if (alpha >= 1 - EPS)
        initial = best_edge; // greedy choice, use the best edge
    else
    {
        // Choose a threshold weight for cutoff
        long long threshold = alpha * (best_edge.w - worst_edge.w) + worst_edge.w;

        // Construct Restricted Candidate List
        std::vector<std::pair<int, int>> candidates;
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
        std::vector<std::pair<long long, long long>> cut_values(n_vertices +
                                                                1); // cut_values[i] = {cut_x, cut_y} for vertex i

        std::pair<int, long long> max_vertex = {-1, -INF};
        for (int v : remaining_vertices)
        {
            cut_values[v] = calculate_contribution(v, ret.x, ret.y);
            if (std::max(cut_values[v].first, cut_values[v].second) > max_vertex.second)
                max_vertex = {v, std::max(cut_values[v].first, cut_values[v].second)};
            min_x = std::min(min_x, cut_values[v].first);
            min_y = std::min(min_y, cut_values[v].second);
            max_x = std::max(max_x, cut_values[v].first);
            max_y = std::max(max_y, cut_values[v].second);
        }

        // Choose a vertex based on alpha and the cut values
        long long min_weight = std::min(min_x, min_y);
        long long max_weight = std::max(max_x, max_y);
        int chosen;
        if (alpha >= 1 - EPS)
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
    return ret;
}

Cut local_search_maxcut(Cut &cut, Result &result)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        result.local_iterations++;
        for (int i = 1; i <= n_vertices && !changed; i++)
        {
            std::pair<long long, long long> contribution = calculate_contribution(i, cut.x, cut.y);
            bool present_in_x = (cut.x.count(i) == 1);

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

std::pair<Cut, long long> grasp_maxcut(Result &result)
{
    long long best_cut_value = -INF;
    clock_t start = clock();
    Cut c;
    int iter = 0;

    auto should_continue = [&]() -> bool {
        if (result.construction_type == GREEDY_1)
            return iter < 1; // greedy, no need to run again and again
        else
            return iter < MIN_ITER;
    };

    while (should_continue())
    {
        if (result.construction_type == SEMI_GREEDY_1 || result.construction_type == GREEDY_1)
            c = semi_greedy_maxcut(); // alpha = 1 will give greedy solution
        else if (result.construction_type == RANDOMIZED)
            c = randomized_maxcut();
        else
            assert(false);

        result.construction_cut_value += c.cut_value(adj_matrix); // will average this over no of GRASP iterations
        c = local_search_maxcut(c, result);

        long long cut_value = c.cut_value(adj_matrix);
        result.local_search_cut_value += cut_value; // will average this over no of GRASP iterations
        if (cut_value > best_cut_value)
            best_cut_value = cut_value;

        iter++;
        std::cerr << result.file_name << " " << enum_to_string(result.construction_type) << " Iter " << iter << " "
                  << cut_value << "\n";
    }
    result.local_iterations =
        (double)result.local_iterations / iter; // average local search iterations per GRASP iteration
    result.local_search_cut_value =
        (double)result.local_search_cut_value / iter; // average local optima per GRASP iteration
    result.construction_cut_value =
        (double)result.construction_cut_value / iter; // average construction cut value per GRASP iteration
    result.GRASP_iterations = iter;
    result.GRASP_cut_value = best_cut_value;
    return {c, best_cut_value};
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    std::string input_file;

    if (argc >= 2)
        input_file = std::string(argv[1]);
    else
    {
        std::cout << "Enter input file name: ";
        std::cin >> input_file;
    }

    std::ifstream in(input_file);
    if (!in)
    {
        std::cout << "Error opening input file\n";
        return 0;
    }

    in >> n_vertices >> n_edges;
    adj_matrix.assign(n_vertices + 1, std::vector<long long>(n_vertices + 1, 0));

    // input vertices are 1-indexed
    for (int i = 0; i < n_edges; i++)
    {
        int u, v;
        long long w;
        in >> u >> v >> w;
        adj_matrix[u][v] += w;
        adj_matrix[v][u] += w;
        if (w > best_edge.w)
            best_edge = {u, v, w};
        if (w < worst_edge.w)
            worst_edge = {u, v, w};
    }

    // Greedy 1
    Result res_greedy_1(input_file, n_vertices, n_edges, GREEDY_1);
    alpha = 1;
    grasp_maxcut(res_greedy_1);
    std::cout << res_greedy_1 << "\n";

    // Semi-Greedy 1
    Result res_semi_greedy_1(input_file, n_vertices, n_edges, SEMI_GREEDY_1);
    alpha = (double)rand() / RAND_MAX;
    grasp_maxcut(res_semi_greedy_1);
    std::cout << res_semi_greedy_1 << "\n";

    // Randomized 1
    Result res_randomized_1(input_file, n_vertices, n_edges, RANDOMIZED);
    grasp_maxcut(res_randomized_1);
    std::cout << res_randomized_1 << "\n";
}