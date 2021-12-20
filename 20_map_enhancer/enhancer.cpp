#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>

typedef std::vector<bool> Enhancer;

Enhancer create_enhancer(const std::string& str) {
    Enhancer enhancer;
    for (char c : str) {
        enhancer.push_back(c == '#');
    }
    return enhancer;
}

typedef std::pair<int, int> Position;
typedef std::set<Position> Map;

Map create_map(const std::vector<std::string>& str) {
    const int n = str.size();
    const int m = str[0].size();
    Map map;

    for (int x=0; x<m; ++x) {
        for (int y=0; y<n; ++y) {
            if (str[y][x] == '#') {
                map.insert(Position(x,y));
            }
        }
    }

    return map;
}

bool in_map(const Map& map, const Position& p) {
    return map.find(p) != map.end();
}

std::pair<Position, Position> map_range(const Map& map) {
    Position min_p(INT_MAX, INT_MAX);
    Position max_p(INT_MIN, INT_MIN);

    for (const auto& [x,y] : map) {
        if (x < min_p.first) {
            min_p.first = x;
        }
        if (y < min_p.second) {
            min_p.second = y;
        }
        if (x > max_p.first) {
            max_p.first = x;
        }
        if (y > max_p.second) {
            max_p.second = y;
        }
    }

    return std::make_pair(min_p, max_p);
}

unsigned int pixel_region_value(const Map& map, const Position& p) {
    unsigned int value = 0;
    for (int j=-1; j<=1; ++j) {
        for (int i=-1; i<=1; ++i) {
            value <<= 1;

            Position q(p.first + i, p.second + j);
            value += in_map(map, q) ? 1 : 0;
        }
    }
    return value;
}

Map evolve_map(const Map& map, const Enhancer& enhancer) {
    auto [min_p, max_p] = map_range(map);
    Map new_map;

    for (int x=min_p.first-2; x<=max_p.first+2; ++x) {
        for (int y=min_p.second-2; y<=max_p.second+2; ++y) {

            Position p(x, y);
            unsigned int value = pixel_region_value(map, p);
            bool pixel = enhancer[value];

            if (pixel) {
                new_map.insert(p);
            }
        }
    }

    return new_map;
}

std::string map_to_string(const Map& map) {
    auto [min_p, max_p] = map_range(map);
    std::string str;

    for (int y=min_p.second-1; y<=max_p.second+1; ++y) {
        for (int x=min_p.first-1; x<=max_p.first+1; ++x) {
            Position p(x, y);
            str += in_map(map, p) ? '#' : '.';
        }
        str += '\n';
    }

    return str;
}

int pixel_count(const Map& map) {
    return map.size();
}

void get_inputs(std::ifstream& input, Enhancer &e, Map &m) {
    std::string line;
    std::getline(input, line);

    e = create_enhancer(line);

    std::getline(input, line);
    std::vector<std::string> str;
    while (std::getline(input, line)) {
        str.push_back(line);
    }

    m = create_map(str);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    Enhancer e;
    Map m;

    std::ifstream input(argv[1]);
    get_inputs(input, e, m);

    std::cout << map_to_string(m) << std::endl;
    std::cout << std::endl;

    m = evolve_map(m, e);
    std::cout << map_to_string(m) << std::endl;
    std::cout << std::endl;

    m = evolve_map(m, e);
    std::cout << map_to_string(m) << std::endl;

    std::cout << "Part 1: " << std::endl;
    std::cout << "      : " << pixel_count(m) << std::endl;
}   