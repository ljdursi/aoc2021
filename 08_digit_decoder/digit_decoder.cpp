#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

const std::map<std::set<char>, int> SegmentsToDigit = {
    {{'a','b','c','e','f','g'}, 0}, 
    {{'c','f'}, 1},
    {{'a','c','d','e','g'}, 2},
    {{'a','c','d','f','g'}, 3},
    {{'b','c','d','f'}, 4},
    {{'a','b','d','f','g'}, 5},
    {{'a','b','d','e','f','g'}, 6},
    {{'a','c','f'}, 7},
    {{'a','b','c','d','e','f','g'}, 8},
    {{'a','b','c','d','f','g'}, 9}
};

std::set<char> apply_wiring_to_segments(const std::string input, const std::vector<char>& wiring) {
    std::set<char> result;

    for (const char c : input) {
        result.insert(wiring[c-'a']);
    }

    return result;
}

bool validDigit(const std::set<char> &segments) {
    return SegmentsToDigit.find(segments) != SegmentsToDigit.end();
}

bool validWiring(const std::vector<char>&wiring, const std::vector<std::string>& inputs) {
    for (const std::string& input : inputs) {
        if (!validDigit(apply_wiring_to_segments(input, wiring))) {
            return false;
        }
    }
    return true;
}

int string_length_compare(const std::string& a, const std::string& b) {
    return a.length() < b.length();
}

std::vector<char> find_wiring(std::vector<std::string>& inputs) {
    std::vector<char> wiring = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

    // the shortest inputs eliminate the most possible wirings - use them first
    std::sort(inputs.begin(), inputs.end(), string_length_compare);

    do {
        if (validWiring(wiring, inputs))
            return wiring;
    } while (std::next_permutation(wiring.begin(), wiring.end()));

    return {};
}

std::vector<int> apply_wiring_to_panel(const std::vector<char>& wiring, const std::vector<std::string>& panels) {
    std::vector<int> result;

    for (const std::string& digit : panels) {
        int decoded_digit = SegmentsToDigit.at(apply_wiring_to_segments(digit, wiring));
        result.push_back(decoded_digit);
    }

    return result;
}

int panel_as_int(const std::vector<int>& panel_ints) {
    int result_int = 0;
    for (int digit : panel_ints) {
        result_int *= 10;
        result_int += digit;
    }
    return result_int;
}

bool parse_input_line(const std::string& line, std::vector<std::string>& inputs, std::vector<std::string>& outputs) {
    const std::string delimiter = "|";
    auto delim_pos = line.find(delimiter);

    if (delim_pos == std::string::npos) {
        std::cout << "line = " << line << std::endl;
        std::cout << "Delim pos = " << delim_pos << std::endl;
        return false;
    }

    std::istringstream ss1(line.substr(0, delim_pos));
    std::copy(std::istream_iterator<std::string>(ss1), std::istream_iterator<std::string>(), back_inserter(inputs));

    std::istringstream ss2(line.substr(delim_pos + delimiter.length(), line.length()));
    std::copy(std::istream_iterator<std::string>(ss2), std::istream_iterator<std::string>(), back_inserter(outputs));

    return true;
}

struct Problem {
    std::vector<std::string> digits;
    std::vector<std::string> panel;
};    

int main() {
    std::vector<struct Problem> problems;
    std::vector<std::vector<int>> decoded_panels;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::vector<std::string> all_digits;
        std::vector<std::string> panel;
        parse_input_line(line, all_digits, panel);

        problems.push_back({all_digits, panel});
        decoded_panels.push_back({0, 0, 0, 0});
    }

    std::transform(problems.begin(), problems.end(), 
                   decoded_panels.begin(),

                   [](struct Problem& problem) {
                    std::vector<char> decoder = find_wiring(problem.digits);
                    return apply_wiring_to_panel(decoder, problem.panel);
                   });
    
    std::cout << "Part 1:" << std::endl;
    int n1478 = 0;
    for (const std::vector<int>& decoded_output : decoded_panels) {
        for (const int digit : decoded_output) {
            if (digit == 1 || digit == 4 || digit == 7 || digit == 8) {
                n1478++;
            }
        }
    }
    std::cout << "    " << n1478 << std::endl;

    std::cout << "Part 2:" << std::endl;
    long outputsum = 0;
    for (const std::vector<int>& decoded_output : decoded_panels) {
        outputsum += panel_as_int(decoded_output);
    }
    std::cout << "    " << outputsum << std::endl;
}