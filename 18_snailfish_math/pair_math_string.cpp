#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <set>
#include <vector>


class SFNumber {
    public:
        SFNumber(std::string input) : s(input) {};
        void add(const SFNumber& b) {
            s = '[' + s + ',' + b.s + ']';
        }
        void reduce();
        std::string to_string() const {
            return s;
        }
        bool apply_split();
        bool apply_explode();
        long int magnitude();
    private:
        std::string s;
        static const int MAX_LEVEL = 4;
        static const int MAX_LITERAL = 9;
        std::vector<std::string> tokenize() const;
        static long int token_magnitude(const std::vector<std::string>& tokens);
};

std::vector<std::string> SFNumber::tokenize() const {
    std::vector<std::string> tokens;
    std::regex tokens_re("(\\[|\\d+|,|\\])");

    auto tokens_begin = std::sregex_iterator(s.begin(), s.end(), tokens_re);
    auto tokens_end = std::sregex_iterator();

    for (auto it = tokens_begin; it != tokens_end; ++it) {
        std::smatch match = *it;
        std::string token = match.str();
        tokens.push_back(token);
    }

    return tokens;
}

long int SFNumber::token_magnitude(const std::vector<std::string>& tokens) {
    if ((tokens.size() == 1)) {
        return std::stol(tokens[0]);
    } else {
        int level = 0, divider=-1;
        for (int i=0; i<tokens.size(); i++) {
            if (tokens[i] == "[") {
                level++;
            } else if (tokens[i] == "]") {
                level--;
            } else if (tokens[i] == "," && level == 1) {
                divider = i;
            }
        }

        // magnitude of [ ... , .... ] is 3*|...| + 2*|....|
        std::vector<std::string> left_tokens(tokens.begin()+1, tokens.begin()+divider);
        std::vector<std::string> right_tokens(tokens.begin()+(divider+1), tokens.end()-1);

        return 3l*token_magnitude(left_tokens) + 2l*token_magnitude(right_tokens);
    }
}

long int SFNumber::magnitude() {
    return token_magnitude(tokenize());
}

bool SFNumber::apply_split() {
    const std::vector<std::string> matches = tokenize();
    std::string result = "";

    bool done_split = false;
    for (const auto& smatch: matches) {
        if (smatch == "[") {
            result += smatch;
        } else if (smatch == ",") {
            result += smatch;
        } else if (smatch == "]") {
            result += smatch;
        } else {
            int n = std::stoi(smatch);
            if (n > MAX_LITERAL && !done_split) {
                done_split = true;
                result += "[" + std::to_string(n/2) + "," + std::to_string((n+1)/2) + "]";
            } else {
                result += smatch;
            }
        }
    }

    if (done_split) {
        s = result;
        return true;
    } else {
        return false;
    }
}

bool SFNumber::apply_explode() {
    std::vector<std::string> smatches = tokenize();
    std::set<std::string> nonliterals={"[", ",", "]"};

    bool found_exploder = false;
    int exploder_idx = -1;
    int level = 0;
    for (int i=0; i<smatches.size(); i++) {
        if (smatches[i] == "[") {
            level++;
        } else if (smatches[i] == "]") {
            level--;
        } else if (smatches[i] == ",") {
            continue;
        } else {
            // this is a number.
            // have we found a number pair?
            bool pair = (nonliterals.find(smatches[i]) == nonliterals.end()) 
                        && (i+1 < smatches.size()) && (smatches[i+1] == ",")
                        && (i+2 < smatches.size()) && (nonliterals.find(smatches[i+2]) == nonliterals.end());
            // should we explode it?
            if (pair && (level > MAX_LEVEL)) {
                found_exploder = true;
                exploder_idx = i-1;
                break;
            }
        }
    }

    if (!found_exploder) {
        return false;
    }

    int left_exploded_val = std::stoi(smatches[exploder_idx+1]);
    int right_exploded_val = std::stoi(smatches[exploder_idx+3]);

    int left_neighbour_idx = -1;
    int right_neighbour_idx = -1;
    for (int i=exploder_idx-1; i>=0; i--) {
        if (nonliterals.find(smatches[i]) == nonliterals.end()) {
            left_neighbour_idx = i;
            break;
        }
    }
    for (int i=exploder_idx+5; i<smatches.size(); i++) {
        if (nonliterals.find(smatches[i]) == nonliterals.end()) {
            right_neighbour_idx = i;
            break;
        }
    }

    std::string result = "";
    int i=0;
    while (i<smatches.size()) {
        if (i == left_neighbour_idx) {
            result += std::to_string(std::stoi(smatches[i]) + left_exploded_val);
            i++;
        } else if (i == right_neighbour_idx) {
            result += std::to_string(std::stoi(smatches[i]) + right_exploded_val);
            i++;
        } else if (i == exploder_idx) {
            // we're replacing the whole '[', num, ',' num, ']' with '0'
            result += "0";
            i += 5;
        } else {
            result += smatches[i];
            i++;
        }
    }

    s = result;
    return true;
}

void SFNumber::reduce() {
    bool changed = false;
    do {
        changed = false;
        if (apply_explode()) {
            changed = true;
            continue;
        }
        if (apply_split()) {
            changed = true;
            continue;
        }
    } while (changed);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    std::string line;
    std::vector<SFNumber> numbers;
    while (std::getline(input, line)) {
        numbers.push_back(SFNumber(line));
    }

    SFNumber result = numbers[0];
    for (int i=1; i<numbers.size(); i++) {
        result.add(numbers[i]);
        result.reduce();
    }

    std::cout << "Part 1: " << std::endl;
    std::cout << "    after addition: " << result.to_string() << std::endl;
    std::cout << "    magnitude     : " << result.magnitude() << std::endl;

    std::cout << "Part 2: " << std::endl;
    long int max_mag = LONG_MIN;
    for (const auto& n1: numbers) {
        for (const auto& n2: numbers) {
            SFNumber n3 = n1;
            n3.add(n2);
            n3.reduce();
            if (n3.magnitude() > max_mag) {
                max_mag = n3.magnitude();
            }
        }
    }
    std::cout << "    max magnitude : " << max_mag << std::endl;
}