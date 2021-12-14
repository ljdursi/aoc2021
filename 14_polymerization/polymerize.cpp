#include <regex>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>

typedef char Elem;
typedef std::pair<Elem,Elem> ElemPair;
typedef std::map<ElemPair, long> ElemPairCounts;

void get_inputs(std::ifstream& input, std::string& initial_polymer, std::map<ElemPair, Elem>& insertion_rules) {
    std::string line;
    bool first_line = true;
    const std::regex insertion_rule_re("([A-Z]+) -> ([A-Z])");
    std::cmatch matches;

    while (std::getline(input, line)) {
        std::smatch matches;
        if (first_line && !line.empty()) {
            initial_polymer = line;
            first_line = false;
            continue;
        }

        if (std::regex_match(line, matches, insertion_rule_re)) {
            std::string key = matches[1];
            std::string value = matches[2];
            insertion_rules[ElemPair(key[0],key[1])] = value[0];
        }
    }
}

std::string to_string(const std::map<ElemPair, Elem>& rules) {
    std::stringstream ss;

    for (auto [key, value] : rules) {
        ss << key.first << key.second << " -> " << value << std::endl;
    }
    return ss.str();
}

void apply_rules_to_string(std::string& s, const std::map<ElemPair, Elem>& insertion_rules) {
    Elem last = '@';
    std::string result;

    for (const Elem c: s) {
        if (insertion_rules.find(ElemPair(last,c)) != insertion_rules.end()) {
            result += insertion_rules.at(ElemPair(last,c));
        }
        result += c;
        last = c;
    }

    s = result;
}

ElemPairCounts string_to_paircounts(const std::string& input) {
    ElemPairCounts counts;

    for (int i=1; i<input.length(); i++) {
        ElemPair pair(input[i-1], input[i]);
        counts[pair]++;
    }

    return counts;
}

std::string to_string(const ElemPairCounts& counts) {
    std::stringstream ss;

    for (auto [key, value] : counts) {
        ss << key.first << key.second << ": " << value << std::endl;
    }
    return ss.str();
}


void apply_rules_to_paircounts(ElemPairCounts& pc, const std::map<ElemPair, Elem>& insertion_rules) {
    ElemPairCounts result;

    for (auto [key, value] : pc) {
        if (insertion_rules.find(key) != insertion_rules.end()) {
            Elem insertion = insertion_rules.at(key);
            char last = key.first, next = key.second;
            for (const ElemPair& e: {ElemPair(last, insertion), ElemPair(insertion, next)}) {
                result[e] += value;
            }
        } else {
            result[key] += value;
        }
    }

    pc = result;
}

long calculate_score(const std::map<Elem, long>& counts) {
    long max_count = LONG_MIN, min_count=LONG_MAX;
    for (auto [key, value] : counts) {
        if (value > max_count) {
            max_count = value;
        }
        if (value < min_count) {
            min_count = value;
        }
    }
    return max_count - min_count;
}

long score_polymer(const std::string& polymer) {
    std::map<Elem, long> counts;
    for (auto c: polymer) {
        counts[c]++;
    }
    return calculate_score(counts);
}

long score_paircounts(const std::string& initial_polymer, const ElemPairCounts& paircounts) {
    std::map<Elem, long> counts;
    for (auto [key, value] : paircounts) {
        counts[key.first] += value;
        counts[key.second] += value;
    }

    // Everything's double counted except the first and last character
    counts[initial_polymer[0]]--;
    counts[initial_polymer[initial_polymer.length()-1]]--;

    for (auto [key, value] : counts)
        counts[key] = value / 2;

    counts[initial_polymer[0]]++;
    counts[initial_polymer[initial_polymer.length()-1]]++;

    return calculate_score(counts);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);    
    std::string initial_polymer;
    std::map<ElemPair, Elem> insertion_rules;

    get_inputs(input, initial_polymer, insertion_rules);
    std::string polymer = initial_polymer;
    std::cout << "Part 1:" << std::endl;
    for (int i=0; i<10; i++) {
        apply_rules_to_string(polymer, insertion_rules);
    }
    std::cout << "      Score: " << score_polymer(polymer) << std::endl;

    // Now, do it smarter for a larger number of iterations
    std::cout << "Part 2:" << std::endl;
    ElemPairCounts paircounts = string_to_paircounts(initial_polymer);
    for (int i=0; i<40; i++) {
        apply_rules_to_paircounts(paircounts, insertion_rules);
    }
    std::cout << "      Score: " << score_paircounts(polymer, paircounts) << std::endl;
}