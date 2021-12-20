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

class Map {
    public:
        Map() : map({}) {}
        Map(const std::vector<std::string>& str) {
            const int n = str.size();
            const int m = str[0].size();

            for (int x=0; x<m; ++x) {
                for (int y=0; y<n; ++y) {
                    if (str[y][x] == '#') {
                        map.insert(Position(x,y));
                    }
                }
            }

            maxx = m-1;
            maxy = n-1;
            border_value = false;
        }

        bool in_map(const Position& pos) const {
            if (within_border(pos)) {
                return map.find(pos) != map.end();
            } else {
                return border_value;
            }
        }

        unsigned int pixel_region_value(const Position& p) const {
            unsigned int value = 0;
            for (int j=-1; j<=1; ++j) {
                for (int i=-1; i<=1; ++i) {
                    value <<= 1;

                    Position q(p.first + i, p.second + j);
                    value += in_map(q) ? 1 : 0;
                }
            }
            return value;
        }

        void evolve(const Enhancer &enhancer) {
            std::set<Position> pixels_to_examine;
            std::set<Position> new_map;

            for (int x = minx-1; x <= maxx+1; x++) {
                for (int y = miny-1; y <= maxy+1; y++) {
                    Position p(x,y);

                    unsigned int value = pixel_region_value(p);
                    bool pixel = enhancer[value];
                    if (pixel)
                        new_map.insert(p);
                }
            }

            map = new_map;

            // expand the grid
            minx--; miny--;
            maxx++; maxy++;

            border_value = (border_value == false ? enhancer[0] : enhancer[511]);
        } 

        std::string to_string() const {
            std::string str;
            for (int y=miny-2; y<=maxy+2; y++) {
                for (int x=minx-2; x<=maxx+2; x++) {
                    Position p(x, y);
                    str += in_map(p) ? '#' : '.';
                }
                str += '\n';
            }
            return str;
        }

        int size() const {
            return map.size();
        }

    private:
        std::set<Position> map;
        int minx = 0, miny = 0;
        int maxx, maxy;
        bool border_value = false;

        bool within_border(const Position& pos) const {
            return pos.first >= minx && pos.first <= maxx && pos.second >= miny && pos.second <= maxy;
        }
};


void get_inputs(std::ifstream& input, Enhancer &e, Map &m) {
    std::string line;
    std::getline(input, line);

    e = create_enhancer(line);

    std::getline(input, line);
    std::vector<std::string> str;
    while (std::getline(input, line)) {
        str.push_back(line);
    }

    m = Map(str);
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

    std::cout << m.size() << std::endl;
    std::cout << m.to_string() << std::endl;

    m.evolve(e);
    std::cout << m.to_string();

    m.evolve(e);
    std::cout << m.to_string();

    std::cout << "Part 1: " << std::endl;
    std::cout << "      : " << m.size() << std::endl;

    for (int i=2; i<50; i++) {
        m.evolve(e);
    }

    std::cout << "Part 2: " << std::endl;
    std::cout << "      : " << m.size() << std::endl;
}   