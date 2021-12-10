#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <set>
    
const std::map<char,char> closes = {{')','('}, {']','['}, {'}','{'}, {'>','<'}};
const std::map<char,int> invalid_points = {{')',3}, {']',57}, {'}',1197}, {'>',25137}};
const std::map<char,int> missing_points = {{')',1}, {']',2}, {'}',3}, {'>',4}};

int main() {
    std::vector<std::string> v;
    std::string input;
    while (std::getline(std::cin, input)) {
        v.push_back(input);
    }

    std::map<char,char> opens;
    for (auto c : closes) {
        opens[c.second] = c.first;
    }

    std::vector<std::vector<char>> incomplete_characters;
    std::vector<char> bad_characters;

    for (auto &str : v) {
        std::stack<char> s;
        bool invalid = false;

        for (auto &c : str) {
            if (opens.find(c) != opens.end()) {
                s.push(c);
            } else if (closes.find(c) != closes.end()) {
                if (s.empty() || s.top() != closes.at(c)) {
                    bad_characters.push_back(c);
                    invalid = true;
                    break;
                } else {
                    s.pop();
                }
            }
        }

        if (invalid)
            continue;

        if (!s.empty()) {
            std::vector<char> incomplete;
            while (!s.empty()) {
                incomplete.push_back(opens.at(s.top()));
                s.pop();
            }
            incomplete_characters.push_back(incomplete);
        }
    }

    int score = 0;
    for (const char c : bad_characters) 
        score += invalid_points.at(c);

    std::cout << "Part 1: " << std::endl;
    std::cout << "      score = " << score << std::endl;

    std::vector<long> scores;
    for (const auto &incomplete : incomplete_characters) {
        long score = 0l;
        for (const char c : incomplete) {
            score *= 5;
            score += missing_points.at(c);
        }
        scores.push_back(score);
    }

    std::sort(scores.begin(), scores.end());
    std::cout << "Part 2: " << std::endl;
    std::cout << "      score = " << scores[scores.size()/2] << std::endl;
}