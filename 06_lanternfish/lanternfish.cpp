#include <iostream>
#include <sstream>
#include <string>
#include <numeric>
#include <vector>

class LanternFishPopulation {
    public:
        LanternFishPopulation(std::vector<int> initial_timers) : maxdays(8) {
            for (int i=0; i<maxdays+1; i++)
                number_at_timer.push_back(0l);

            for (const int time: initial_timers) {
                number_at_timer[time]++;
            }
        }
        long size() const {
            long sum = 0;
            for (const long daycount: number_at_timer)
                sum += daycount;
            return sum;
        }
        std::string to_string() const {
            std::stringstream ss;
            for (int i=0; i<maxdays; i++) {
                ss << i << ": " << number_at_timer[i] << ", ";
            }
            ss << maxdays << ": " << number_at_timer[maxdays];
            return ss.str(); 
        }
        void evolve() {
            auto nzero = number_at_timer[0];
            for (int i=0; i<maxdays; i++)
                number_at_timer[i] = number_at_timer[i+1];

            number_at_timer[maxdays] = nzero;
            number_at_timer[maxdays-2] += nzero;
        }
    private:
        const int maxdays;
        std::vector<long> number_at_timer;
};

int main() {
    std::vector<int> fish_days;

    std::string line;
    std::cin >> line;
    std::stringstream ss(line);
    std::string number;
    while (std::getline(ss, number, ',')) {
        fish_days.push_back(std::stoi(number));
    }

    LanternFishPopulation population(fish_days);

    std::cout << "Part 1: " << std::endl;
    int day = 0;
    for (; day<80; day++) 
        population.evolve();
    std::cout << "  Population after 80 days " << population.size() << std::endl;

    std::cout << "Part 2: " << std::endl;
    for (; day<256; day++) {
        population.evolve();
    }
    std::cout << "  Population after 256 days " << population.size() << std::endl;
}