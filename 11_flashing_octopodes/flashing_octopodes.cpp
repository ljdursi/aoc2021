#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class OctopusGrid {
    public:
        OctopusGrid(std::istream& input);
        std::string to_string() const;
        int n_flashes() const;
        void evolve();
        bool all_flashed() const; // did all flash in the last round?
    private:
        std::vector<std::vector<short int>> grid;
        std::vector<std::pair<int, int>> neighbours(const std::pair<int, int>& pos) const;
        int n, m;
        int nflashes;
        bool allflashed;
};

OctopusGrid::OctopusGrid(std::istream& input) : nflashes(0), allflashed(false) {
    std::string line;

    while (std::getline(input, line)) {
        std::vector<short int> row;
        for (char c : line) {
            row.push_back(c - '0');
        }
        grid.push_back(row);
    }

    n = grid.size();
    m = grid[0].size();
}

std::string OctopusGrid::to_string() const{
    std::stringstream output;
    for (auto row : grid) {
        for (auto c : row) {
            output << c;
        }
        output << std::endl;
    }
    return output.str();
}

int OctopusGrid::n_flashes() const {
    return nflashes;
}

bool OctopusGrid::all_flashed() const {
    return allflashed;
}

std::vector<std::pair<int, int>> OctopusGrid::neighbours(const std::pair<int, int>& pos) const {
    std::vector<std::pair<int, int>> valid_neighbours;

    for (int i=-1; i<=+1; i++) {
        for (int j=-1; j<=+1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            if (pos.first + i >= 0 && pos.first + i < n && pos.second + j >= 0 && pos.second + j < m) 
                valid_neighbours.push_back(std::make_pair(pos.first + i, pos.second + j));
        }
    }

    return valid_neighbours;
}

void OctopusGrid::evolve() {
    // everyone gets some energy...
    for (auto &row : grid) {
        std::transform(row.begin(), row.end(), row.begin(), [](auto energy){return energy+1;});
    }

    // let the flashing begin.
    bool flashes_this_cycle;
    std::vector<std::pair<int, int>> flashers;
    do {
        flashes_this_cycle = false;
        
        // find the flashers and increase their neighbour's energy
        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < grid[i].size(); j++) {
                if (grid[i][j] > 9) {
                    grid[i][j] = 0;
                    auto coords = std::make_pair(i, j);
                    flashers.push_back(coords);
                    for (auto neighbour: neighbours(coords)) {
                        grid[neighbour.first][neighbour.second]++;
                    }
                    flashes_this_cycle = true;
                }
            }
        }
    } while (flashes_this_cycle);

    for (auto &flasher : flashers) {
        grid[flasher.first][flasher.second] = 0;
    }

    nflashes += flashers.size();
    allflashed = (flashers.size() == n*m);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    OctopusGrid grid(input);

    std::cout << "Part 1:" << std::endl;

    int step = 0, first_allflash = -1;
    for (; step < 100; step++) {
        grid.evolve();
        if (grid.all_flashed() and first_allflash == -1) {
            first_allflash = step+1;
        }
    }
    std::cout << "   after 100 steps: " << grid.n_flashes() << std::endl;

    std::cout << "Part 2:" << std::endl;
    for (; first_allflash == -1; step++) {
        grid.evolve();
        if (grid.all_flashed() and first_allflash == -1) {
            first_allflash = step+1;
        }
    }
    std::cout << "     first synchronized flash = " << first_allflash << std::endl;
}