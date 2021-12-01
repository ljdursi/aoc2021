#include <iostream>
#include <vector>

int depths_increasing(const std::vector<int> depths) {
    int nincreasing = 0;
    for (int i=1; i<depths.size(); i++) {
        if (depths[i] > depths[i-1]) {
            nincreasing++;
        }
    }
    return nincreasing;
}

std::vector<int> sliding_window(const std::vector<int> points, const int width) {
    const int nwindows = points.size()-width+1;
    std::vector<int> results(nwindows);

    for (int i=0; i<nwindows; i++) {
        for (int j=0; j<width; j++) {
            results[i] += points[i+j];
        }
    }

    return results;
}

int main() {
    std::vector<int> depths;
    int depth;
    while (std::cin >> depth) {
        depths.push_back(depth);
    }

    std::cout << "Part 1: " << depths_increasing(depths) << std::endl;
    std::vector<int> windows = sliding_window(depths, 3);
    std::cout << "Part 2: " << depths_increasing(windows) << std::endl;
}