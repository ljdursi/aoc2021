#include <regex>
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>

typedef std::vector<std::vector<short int>> Grid;
typedef std::pair<short int, short int> Coord;

Grid get_inputs(std::ifstream& input) {
    Grid result;
    std::string line;
    while (std::getline(input, line)) {
        std::vector<short int> row;
        for (const char c: line) {
            row.push_back(c-'0');
        }
        result.push_back(row);
    }

    return result;
}

std::string to_string(const Grid& g) {
    std::stringstream ss;
    for (const auto& row: g) {
        for (const auto& c: row) {
            ss << c;
        }
        ss << std::endl;
    }

    return ss.str();
}

std::string to_string(const Grid& g, const std::vector<Coord>& path) {
    const int n = g.size();
    const int m = g[0].size();

    std::vector<std::vector<char>> rows(n);
    for (int i=0; i<n; i++) {
        rows[i] = std::vector<char>(m,'.');
    }

    for (const auto& c: path) {
        rows[c.first][c.second] = '0'+g[c.first][c.second];
    }

    std::stringstream ss;
    for (const auto& row: rows) {
        for (const char c: row) {
            ss << c;
        }
        ss << std::endl;
    }

    return ss.str();
}

std::vector<Coord> neighbours(Coord c, int n, int m) {
    std::vector<Coord> result;
    for (const Coord& neigh: {Coord(-1,0), Coord(0,-1), Coord(+1,0), Coord(0,+1)}) {
        Coord new_coord = {c.first+neigh.first, c.second+neigh.second};
        if (new_coord.first >= 0 && new_coord.first < n && new_coord.second >= 0 && new_coord.second < m) {
            result.push_back(new_coord);
        }
    }
    return result;
}

Grid bigger_grid(const Grid& g) {
    const int n = g.size();
    const int m = g[0].size();

    Grid result(5*n, std::vector<short int>(5*m, 0));
    for (int i=0; i<n; i++) {
        for (int j=0; j<m; j++) {
            int value = g[i][j];
            for (int ishift=0; ishift<5; ishift++) {
                for (int jshift=0; jshift<5; jshift++) {
                    result[i+n*ishift][j+m*jshift] = (value-1+ishift+jshift)%9+1;
                }
            }
        }
    }
    return result;
}

// Dijkstra's algorithm, which seems overkill here?
int find_min_path(const Grid& g, std::vector<Coord>& path) {
    const int n=g.size();
    const int m=g[0].size();

    std::vector<std::vector<Coord>> prev(n, std::vector<Coord>(m, Coord(-1,-1)));
    std::vector<std::vector<int>> dist(n, std::vector<int>(m, INT_MAX));
    std::vector<std::vector<bool>> dequeued(n, std::vector<bool>(m,false));
    std::vector<std::vector<bool>> queued(n, std::vector<bool>(m,false));

    auto cmp = [&dist](Coord left, Coord right) {return dist[left.first][left.second] > dist[right.first][right.second];};
    std::vector<Coord> coords;

    dist[0][0] = 0;
    coords.push_back(Coord(0,0));

    while (!coords.empty()) {
        std::sort(coords.begin(), coords.end(), cmp);
        const Coord c = coords.back();
        coords.pop_back();

        for (auto neigh: neighbours(c, n, m)) {
            if (dequeued[neigh.first][neigh.second]) {
                continue;
            }
            if (dist[neigh.first][neigh.second] > dist[c.first][c.second] + g[neigh.first][neigh.second]) {
                dist[neigh.first][neigh.second] = dist[c.first][c.second] + g[neigh.first][neigh.second];
                prev[neigh.first][neigh.second] = c;
                if (!queued[neigh.first][neigh.second]) {
                    coords.push_back(neigh);
                    queued[neigh.first][neigh.second] = true;
                }
            }
        }
        dequeued[c.first][c.second] = true;
    }

    // backtrack
    path.push_back(Coord(n-1,m-1));
    while (prev[path.back().first][path.back().second] != Coord(-1,-1)) {
        path.push_back(prev[path.back().first][path.back().second]);
    }
    return dist[n-1][m-1];
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);    
    const Grid grid = get_inputs(input);

    std::vector<Coord> path;
    const int min_path = find_min_path(grid, path);

    std::cout << "Part 1:" << std::endl;
    std::cout << "      Min cost path = " << min_path << std::endl;

    std::cout << "Part 2:" << std::endl;
    Grid big_grid = bigger_grid(grid);

    path.clear();
    const int min_big_path = find_min_path(big_grid, path);
    std::cout << "      Min cost path = " << min_big_path << std::endl;
}