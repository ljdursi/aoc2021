#include <iostream>
#include <vector>
#include <string>

std::pair<int, int> gamma_epsilon(std::vector<std::vector<int>> binary) {
    int gamma = 0, epsilon = 0;
    const int n = binary.size();

    if (n == 0) {
        return std::make_pair(gamma, epsilon);
    }

    const int m = binary[0].size();
    for (int j = 0; j < m; j++) {
        gamma <<= 1;
        epsilon <<= 1;
        int n_zero=0;
        for (int i=0; i<n; i++) {
            if (binary[i][j] == 1)
                n_zero ++;
        }
        if (n_zero > (n-n_zero)) {
            gamma += 0;
            epsilon += 1;
        } else {
            gamma += 1;
            epsilon += 0;
        }
    }

    return std::make_pair(gamma, epsilon);
}

int oxygen_or_co2(std::vector<std::vector<int>> binary, bool use_most_common) {
    const int n = binary.size();
    int result = 0;

    if (n == 0) 
        return result;

    const int m = binary[0].size();
    std::vector<bool> keep(n, true);

    int nremaining = n;
    for (int j = 0; (j < m) && (nremaining > 1); j++) {
        int n_zero=0;
        for (int i=0; i<n; i++) {
            if (keep[i] && binary[i][j] == 0)
                n_zero ++;
        }

        int most_common = n_zero > (nremaining-n_zero) ? 0 : 1;
        for (int i=0; i<n; i++) {
            if (keep[i]) {
                bool survives_round = (binary[i][j] == most_common) == use_most_common;
                if (!survives_round) {
                    keep[i] = false;
                    nremaining--;
                }
            }
        }
    }

    auto found = std::find(keep.begin(), keep.end(), true);
    if (found == keep.end()) {
        return result;
    }

    const int idx = std::distance(keep.begin(), found);
    for (const int item : binary[idx]) {
        result <<= 1;
        result += item;
    }

    return result;
}

int main() {
    std::string binaryline;
    std::vector<std::vector<int>> binary;

    while (std::cin >> binaryline) {
        std::vector<int> bits;
        for (char c : binaryline) {
            bits.push_back(c - '0');
        }
        binary.push_back(bits);
    }

    std::pair<int, int> result = gamma_epsilon(binary);
    std::cout << "Part 1:" << std::endl;
    std::cout << "  Gamma = " << result.first << std::endl;
    std::cout << "  Epsilon = " << result.second << std::endl;

    std::cout << std::endl;
    std::cout << "Part 2:" << std::endl;
    const int o2 = oxygen_or_co2(binary, true);
    std::cout << "  O2 Generator = " << o2 << std::endl;
    const int co2 = oxygen_or_co2(binary, false);
    std::cout << "  CO2 Scrubber = " << co2 << std::endl;
    std::cout << "  Life Support Rating = " << o2*co2 << std::endl;
}
