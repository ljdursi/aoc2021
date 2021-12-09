#include <iostream>
#include <vector>
#include <string>
#include <queue>

std::vector<std::vector<int>> read_map() {
    std::vector<std::string> lines;
    std::string line;
    int nlines = 0;
    while (std::cin >> line) {
        lines.push_back(line);
        nlines++;
    }

    std::vector<std::vector<int>> map(nlines);
    for (int i = 0; i < nlines; i++ ) {
        for (int j = 0; j < lines[i].length(); j++) {
            map[i].push_back(lines[i][j] - '0');
        }
    }

    return map;
}

std::vector<std::pair<int,int>> local_minima(std::vector<std::vector<int>> &map) {
    std::vector<std::pair<int,int>> local_mins;

    std::vector<int> neighbour_heights(4, 0);
    for (int i = 0; i < map.size(); i++) {
        for (int j = 0; j < map[i].size(); j++) {
            int nneigh = 0;
            if (i > 0)
                neighbour_heights[nneigh++] = map[i - 1][j];
            if (i < map.size() - 1) 
                neighbour_heights[nneigh++] = map[i + 1][j];
            if (j > 0) 
                neighbour_heights[nneigh++] = map[i][j-1];
            if (j < map[i].size() - 1)
                neighbour_heights[nneigh++] = map[i][j+1];

            if (map[i][j] < *std::min_element(neighbour_heights.begin(), neighbour_heights.begin() + nneigh)) {
                local_mins.push_back(std::make_pair(i, j));
            }
        }
    }

    return local_mins;
}

std::vector<int> floodfill_basins(const std::vector<std::vector<int>>& map, const std::vector<std::pair<int,int>>& local_mins) {
    std::vector<int> basin_sizes;

    const int n = map.size();
    const int m = map[0].size();
    char basin_ids[n][m];

    for (int i=0; i<n; i++) {
        for (int j=0; j<m; j++) {
            if (map[i][j] == 9)
                basin_ids[i][j] = '*';
            else 
                basin_ids[i][j] = '.';
        }
    }

    const int nminima = local_mins.size();

    for (int min_id = 0, basin_id=0; min_id < nminima; min_id++) {
        int i = local_mins[min_id].first;
        int j = local_mins[min_id].second;

        if (basin_ids[i][j] != '.') // already subsumed by another basin
            continue;

        int basin_size = 0;

        std::queue<std::pair<int,int>> q;
        q.push(local_mins[min_id]);
        while (!q.empty()) {
            auto coords = q.front(); q.pop();
            int i = coords.first;
            int j = coords.second;

            if (basin_ids[i][j] != '.')
                continue;

            basin_ids[i][j] = basin_id + '0';
            basin_size++;
            if (i > 0 && basin_ids[i-1][j] == '.')
                q.push(std::make_pair(i-1, j));
            if (i < n-1 && basin_ids[i+1][j] == '.')
                q.push(std::make_pair(i+1, j));
            if (j < m-1 && basin_ids[i][j+1] == '.')
                q.push(std::make_pair(i, j+1));
            if (j > 0 && basin_ids[i][j-1] == '.')
                q.push(std::make_pair(i, j-1));
        }
        basin_sizes.push_back(basin_size);
        basin_id++;
    }

    return basin_sizes;
}

int main() {
    auto map = read_map();
    auto minima = local_minima(map);

    std::cout << "Part 1:" << std::endl;
    int risk_level_sum = 0;
    for (const auto &min: minima) {
        risk_level_sum += map[min.first][min.second] + 1;
    }
    std::cout << "     " << risk_level_sum << std::endl;

    auto basin_sizes = floodfill_basins(map, minima);
    std::sort(basin_sizes.begin(), basin_sizes.end(), std::greater<int>());

    std::cout << "Part 2:" << std::endl;
    std::cout << "     " << basin_sizes[0]*basin_sizes[1]*basin_sizes[2] << std::endl;
}
