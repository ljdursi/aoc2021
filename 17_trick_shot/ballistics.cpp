#include <regex>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

void get_inputs(std::ifstream& input, int& xmin, int& xmax, int& ymin, int& ymax) {
    std::string line;
    const std::regex target_re("target area: x=([-]?\\d+)..([-]?\\d+), *y=([-]?\\d+)..([-]?\\d+)");
    std::smatch smatches;

    while (true) {
        std::getline(input, line);
        if (!std::regex_match(line, smatches, target_re))
            continue;

        xmin = std::stoi(smatches[1]);
        xmax = std::stoi(smatches[2]);
        ymin = std::stoi(smatches[3]);
        ymax = std::stoi(smatches[4]);
        break; 
    }
}

void x_velocity_range(const int x_min, const int x_max, int &vx_min, int &vx_max) {
    // go any faster than this, and we'll be past the target after the first second
    vx_max = x_max;

    // go any slower than this, and we'll never get to the target
    int abs_posn = abs(x_min);

    int vx = 0;
    int max_pos = vx_min*(vx_min+1)/2;
    while (max_pos < abs_posn) {
        vx++;
        max_pos = vx*(vx+1)/2;
    }

    vx_min = vx;
    if (x_min < 0)
        vx_min = -vx_min;
}

std::vector<int> possible_factors(const int n) {
    // factors of n 
    std::vector<int> factors;
    const int an = abs(n);
    for (int i=1; i<=an; i++) {
        if (an % i == 0)
            factors.push_back(i);
    }
    return factors;
}

int xpos(const int vx0, const int t) {
    // The discrete equation of motion in x is:
    // x(t) = vx0*t + t(t-1)/2*a, t < |vx0|
    //   where a = -1 if vx0 > 0, 0 if vx0 = 0, +1 if vx0 < 0
    //   when t = vx0, vx = 0 and x remains constant

    int a = (vx0 > 0) ? -1 : (vx0 < 0) ? 1 : 0;
    int tmax = abs(vx0);
    int xmax = vx0*tmax + tmax*(tmax-1)/2*a;

    if (t > tmax)
        return xmax;

    int x = vx0*t + t*(t-1)/2*a;
    return x;
}

int ypos(const int vy0, const int t) {
    // The discrete equation of motion in y is:
    // y(t) = t*vy0 - t(t-1)/2
    return t*vy0 - t*(t-1)/2;
}

std::vector<std::pair<int, int>> valid_vy0_t_pairs(const int y_min, const int y_max) {
    // The discrete equation of motion here is:
    // y(t) = t*vy0 - t(t-1)/2
    // 
    // To find solutions that fall within the target area (y_min, y_max), we solve the
    // equation for vy0:
    //
    // vy0 = v_target/t + (t-1)/2
    // 
    // For that to have an integer solution, t has to a factor of v_target*2, or 
    // So for every valid target y, we find valid (vy0, target) pairs.
    std::vector<std::pair<int, int>> result;
    int dy = y_max > y_min ? 1 : -1;

    for (int yt=y_min; yt<=y_max; yt+=dy) {
        for (const int t: possible_factors(2*yt)) {
            int vy0 = yt/t + (t-1)/2;
            if (ypos(vy0, t) == yt)
                result.push_back({vy0, t});
        }
    }
    return result;
}

std::set<std::pair<int, int>> valid_initial_conditions(const int x_min, const int x_max, const int y_min, const int y_max) {
    // solve for y part of the equation of motion first
    auto vy0_t_pairs = valid_vy0_t_pairs(y_min, y_max);

    // now we'll see if there are any valid x initial conditions
    int vx_min, vx_max;
    x_velocity_range(x_min, x_max, vx_min, vx_max);

    std::set<std::pair<int, int>> result;
    for (int vx0 = vx_min; vx0 <= vx_max; vx0++) {
        for (const auto& vy0_t: vy0_t_pairs) {
            int vy0 = vy0_t.first, t = vy0_t.second;

            // for the trial vx0, does x(vx0, t) fall within the target area?
            int x = xpos(vx0, t);
            if (x >= x_min && x <= x_max)
                result.insert({vx0, vy0});
        }
    }

    return result;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    int xmin, xmax, ymin, ymax;

    get_inputs(input, xmin, xmax, ymin, ymax);
    auto solutions = valid_initial_conditions(xmin, xmax, ymin, ymax);

    int max_ypos = 0;
    for (const auto& soln: solutions) {
        int vy0 = soln.second; 
        if (vy0 <= 0) // max_ypos = 0;
            continue;

        // the peak position is where vy = 0, and since vy = vy0 - t
        // that's where t = vy0. 
        int peak_ypos =  ypos(vy0, vy0);

        if (peak_ypos > max_ypos)
            max_ypos = peak_ypos;
    }

    std::cout << "Part 1:" << std::endl;
    std::cout << "     max_ypos = " << max_ypos << std::endl;

    std::cout << "Part 2:" << std::endl;
    std::cout << "     number of valid initial velocities = " << solutions.size() << std::endl;

    return 0;
}