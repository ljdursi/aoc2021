#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <regex>

typedef std::pair<int, int> Point;
typedef std::set<Point> Sheet;

enum struct FoldDir {
    X = 1,
    Y = 2
};

typedef std::pair<FoldDir, int> Fold;

std::string print_sheet(const Sheet& sheet) {
    int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
    for (const auto& point : sheet) {
        if (point.first < minx) minx = point.first;
        if (point.second < miny) miny = point.second;
        if (point.first > maxx) maxx = point.first;
        if (point.second > maxy) maxy = point.second;
    }

    std::stringstream ss;
    for (int y=miny; y<=maxy; y++) {
        for (int x=minx; x<=maxx; x++) {
            if (sheet.find(Point(x, y)) != sheet.end()) {
                ss << '#';
            } else {
                ss << ' ';
            }
        }
        ss << std::endl;
    }
    return ss.str();
}

void fold_sheet_y(Sheet& sheet, int yfold) {
    Sheet new_sheet;

    for (const auto& [x,y]: sheet) {
        if (y > yfold) 
            new_sheet.insert(Point(x, 2*yfold-y));
        else
            new_sheet.insert(Point(x, y));
    }

    sheet.swap(new_sheet);
}

void fold_sheet_x(Sheet& sheet, int xfold) {
    Sheet new_sheet;

    for (const auto& [x,y]: sheet) {
        if (x > xfold) 
            new_sheet.insert(Point(2*xfold-x, y));
        else
            new_sheet.insert(Point(x, y));
    }

    sheet.swap(new_sheet);
}

void apply_fold(Sheet& sheet, const Fold& fold) {
    switch (fold.first) {
        case FoldDir::X:
            fold_sheet_x(sheet, fold.second);
            break;
        case FoldDir::Y:
            fold_sheet_y(sheet, fold.second);
            break;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);    
    std::string line;
    Sheet s;
    std::vector<Fold> folds;

    while (std::getline(input, line)) {
        std::cout << "Reading line: " << line << std::endl;
        const std::regex point("(\\d+),(\\d+)");
        const std::regex fold("fold along (x|y)=(\\d+)");
        std::smatch matches;

        if (std::regex_match(line, matches, point)) {
            auto pt = Point(std::stoi(matches[1]), std::stoi(matches[2]));
            s.insert(pt);
        } else if (std::regex_match(line, matches, fold)) {
            auto dir = matches[1] == "x" ? FoldDir::X : FoldDir::Y;
            auto fold_size = std::stoi(matches[2]);
            folds.push_back(Fold(dir, fold_size));
        }
    }

    std::cout << "Part 1:" << std::endl;
    apply_fold(s, folds[0]);
    std::cout << "      Number of points = " << s.size() << std::endl;

    std::cout << "Part 2:" << std::endl;
    for (int i=1; i<folds.size(); i++) {
        apply_fold(s, folds[i]);
    }

    std::cout << print_sheet(s) << std::endl;
}