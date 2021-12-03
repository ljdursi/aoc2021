#include <vector>
#include <map>
#include <iostream>
#include <string>

enum struct Direction {
    UP,
    DOWN,
    FORWARD
};

class Submarine {
    public:
        Submarine(int horiz_start, int depth_start, bool use_aim) : horizontal(horiz_start), depth(depth_start), aim(0), depth_via_aim(use_aim) {};
        void move(Direction direction, int distance);
        int get_depth();
        int get_horizontal();
    private:
        int horizontal;
        int depth; 
        int aim;
        bool depth_via_aim;
};

int Submarine::get_depth() {
    return depth;
}

int Submarine::get_horizontal() {
    return horizontal;
}

void Submarine::move(Direction direction, int distance) {
    switch (direction) {
        case Direction::UP:
            distance = -distance;
        case Direction::DOWN:
            if (depth_via_aim) {
                aim += distance;
            } else {
                depth += distance;
            }
            break;
        case Direction::FORWARD:
            horizontal += distance;
            if (depth_via_aim)
                depth += aim*distance;
            break;
    }
}

int main() {
    const std::map<std::string, Direction> from_string = {{"forward", Direction::FORWARD},
                                                          {"up", Direction::UP},
                                                          {"down", Direction::DOWN}};

    std::vector<std::pair<Direction, int>> moves;

    std::string dirstr;
    int distance;
    while (std::cin >> dirstr >> distance) {
        moves.push_back(std::make_pair(from_string.at(dirstr), distance));
    }

    for (const bool use_aim : {false, true}) {
        std::cout << "Use_aim = " << use_aim << std::endl;
        Submarine sub(0, 0, use_aim);

        for (auto move : moves) 
            sub.move(move.first, move.second);

        std::cout << "   depth: " << sub.get_depth() << " horizontal: " << sub.get_horizontal() << std::endl;
        std::cout << "   area: " << sub.get_horizontal()*sub.get_depth() << std::endl;

        std::cout << std::endl;
    }
}