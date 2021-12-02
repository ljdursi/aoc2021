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
        Submarine(int horiz_start, int depth_start);
        void move(Direction direction, int distance);
        int get_depth();
        int get_horizontal();
    private:
        int horizontal;
        int depth; 
        int aim;
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
            aim -= distance;
            break;
        case Direction::DOWN:
            aim += distance;
            break;
        case Direction::FORWARD:
            horizontal += distance;
            depth += aim*distance;
            break;
    }
}

Submarine::Submarine(int horiz_start, int depth_start) {
    horizontal = horiz_start;
    depth = depth_start;
};

int main() {
    const std::map<std::string, Direction> from_string = {{"forward", Direction::FORWARD},
                                                          {"up", Direction::UP},
                                                          {"down", Direction::DOWN}};

    Submarine sub(0, 0);
    std::string dirstr;
    int distance;
    while (std::cin >> dirstr >> distance) {
        sub.move(from_string.at(dirstr), distance);
    }

    std::cout << "Part 1: " << std::endl;
    std::cout << "   depth: " << sub.get_depth() << " horizontal: " << sub.get_horizontal() << std::endl;
    std::cout << "   area: " << sub.get_horizontal()*sub.get_depth() << std::endl;
}