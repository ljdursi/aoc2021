#include <vector>
#include <numeric>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>

std::vector<int> get_comma_separated_ints(std::string line) {
    std::vector<int> result;
    std::stringstream ss(line);

    std::string number;
    while (std::getline(ss, number, ',')) 
        result.push_back(std::stoi(number));

    return result;
}        

float median(std::vector<int> numbers) {
    int size = numbers.size();
    if (size == 0)
        return 0;

    std::sort(numbers.begin(), numbers.end());

    if (size % 2 == 0)
        return (numbers[size / 2 - 1] + numbers[size / 2]) / 2.0;
    else
        return numbers[size / 2];
}

int fuel_to(std::vector<int> crab_positions, int new_position, bool constant) {
    int result = 0;
    for (int crab_position : crab_positions) {
        int dist = abs(new_position - crab_position);
        if (constant)
            result += dist;
        else 
            result += dist*(dist+1)/2;
    }

    return result;
}

int minfuel_const(std::vector<int> crab_positions) {
    float median_position = median(crab_positions);
    if (median_position == int(median_position))
        return fuel_to(crab_positions, int(median_position), true);
    else {
        int fuel_to_left = fuel_to(crab_positions, floor(median_position), true);
        int fuel_to_right = fuel_to(crab_positions, ceil(median_position), true);
        return std::min(fuel_to_left, fuel_to_right);    
    }
}

int minfuel_linear_brute_force(std::vector<int> crab_positions) {
    // the minimum fuel in this case falls somwhere between the median and the mean, inclusive
    const int n=crab_positions.size();

    float med = median(crab_positions);
    float mean = std::accumulate(crab_positions.begin(), crab_positions.end(), 0) / float(n);

    int min_pos = std::min(floor(med), floor(mean));
    int max_pos = std::max(ceil(med), ceil(mean));

    int min_fuel = INT_MAX;
    for (int pos=min_pos; pos <= max_pos; pos++) {
        int fuel = fuel_to(crab_positions, pos, false);
        if (fuel < min_fuel)
            min_fuel = fuel;
    }

    return min_fuel;
}

int main() {
    std::string line;
    std::cin >> line;

    std::vector<int> crab_positions = get_comma_separated_ints(line);
    std::cout << "Part 1:" << std::endl;
    std::cout << "     " << minfuel_const(crab_positions) << std::endl;

    std::cout << "Part 2:" << std::endl;
    std::cout << "     " << minfuel_linear_brute_force(crab_positions) << std::endl;
}
