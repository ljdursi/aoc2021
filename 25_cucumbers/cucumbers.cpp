#include <map>
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

typedef std::pair<short unsigned int, short unsigned int> Coordinates;

enum struct Cucumber { SOUTH, EAST };

class Board {
    public:
        Board(const std::vector<std::string> lines);
        unsigned int move_cucumbers();
        friend std::ostream& operator<<(std::ostream& os, const Board& b);
        char at(const Coordinates& c) const { 
            const auto it = board.find(c);
            if (it == board.end()) {
                return '.';
            } else {
                return (*it).second == Cucumber::SOUTH ? 'v' : '>';
            }
        };
        void debug_output() const {
            for (const auto& [coord, cucumber] : board) {
                std::cout << coord.first << "," << coord.second << ": " << (cucumber == Cucumber::SOUTH ? "SOUTH" : "EAST") << std::endl;
            }
        }

    private:
        bool empty(const Coordinates& coordinates) const {
            return board.find(coordinates) == board.end();
        };
        std::map<Coordinates, Cucumber> board;
        Coordinates max_coordinates;
};

Board::Board(const std::vector<std::string> lines) {
    const int m = lines.size();
    const int n = lines[0].size();

    max_coordinates = Coordinates(n, m);

    int j = 0;
    for (const auto &line: lines) {
        int i=0;
        for (const char c: line) {
            if (c == 'v') {
                board[Coordinates(i, j)] = Cucumber::SOUTH;
            } else if (c == '>') {
                board[Coordinates(i, j)] = Cucumber::EAST;
            }
            i++;
        }
        j++;
    }
}

unsigned int Board::move_cucumbers() {
    int nmoved = 0;
    auto max_east = max_coordinates.first;
    auto max_south = max_coordinates.second;

    const auto move_east = [max_east](const Coordinates& c) { return Coordinates((c.first + 1) % max_east, c.second); };
    const auto move_south = [max_south](const Coordinates& c) { return Coordinates(c.first, (c.second + 1) % max_south); };

    const auto move = [move_east, move_south](const Cucumber& type, const Coordinates& c) { return type == Cucumber::EAST ? move_east(c) : move_south(c); };

    for (const auto& direction: std::vector<Cucumber>{Cucumber::EAST, Cucumber::SOUTH}) {
        std::map<Coordinates, Cucumber> new_board;
        for (const auto& [coordinates, type]: board) {

            auto new_coords = move(direction, coordinates);
            if ((type == direction) && (this->empty(new_coords))) {
                new_board[new_coords] = type;
                nmoved++;
            } else {
                new_board[coordinates] = type;
            }
        }
        board = new_board;
    }

    return nmoved;
}

std::ostream& operator<<(std::ostream& os, const Board& b) {
    for (int j=0; j<b.max_coordinates.second; j++) {
        for (int i=0; i<b.max_coordinates.first; i++) {
            os << b.at(Coordinates(i, j));
        }
        os << std::endl;
    }
    return os;
};

std::vector<std::string> get_inputs(std::ifstream& input) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }
    return lines;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Could not open input file " << argv[1] << std::endl;
        return 2;
    }

    auto lines = get_inputs(input);
    auto board = Board(lines);  

    std::cout << std::endl;

    unsigned int nmoves = 0;
    std::set<int> output_moves({0,1,2,3,4,5,10,20,30,40,50,55,56,57,58});
    for (;nmoves < 59; nmoves++) {
        if (output_moves.find(nmoves) != output_moves.end()) {
            std::cout << "After " << nmoves << " steps:" << std::endl;
            std::cout << board << std::endl;
        }
        board.move_cucumbers();
    }

    std::cout << "Part 1:" << std::endl;
    while (board.move_cucumbers() > 0) 
        nmoves++;

    std::cout << "    : " << nmoves+1 << std::endl;
}