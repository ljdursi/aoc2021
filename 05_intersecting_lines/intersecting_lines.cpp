#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <map>

class Point {
    public:
        Point() : x(0), y(0) {}
        Point(int x, int y) : x(x), y(y) {}
        Point(std::string text);
        std::string toString() const;
        int x;
        int y;

        bool operator< (const Point& other) const {
            if (x == other.x) {
                return y < other.y;
            }
            return x < other.x;
        }
};

Point::Point(std::string text) {
    std::stringstream ss(text);
    ss >> x;
    ss.ignore();
    ss >> y;
}

std::string Point::toString() const {
    std::stringstream ss;
    ss << x << ", " << y;
    return ss.str();
}

class Line {
    public:
        Line(int x1, int y1, int x2, int y2) {
            start = Point{x1, y1};
            end = Point{x2, y2};

            if (end < start) {
                std::swap(start, end);
            }
        };
        Line(std::string line);
        int signedArea2(const Point p) const;
        bool is_collinear(const Point p) const;
        bool is_collinear(const Line other) const;
        bool is_left(const Point p) const {
            return signedArea2(p) > 0;
        }
        bool is_grid_aligned() const {
            return (start.x == end.x) || (start.y == end.y);
        }
        bool contains_point(const Point p) const;
        std::vector<Point> intersection(const Line other) const;
        std::string toString() const;

        std::vector<Point> get_points() const;
        Point start, end;

};

std::string Line::toString() const {
    std::stringstream ss;
    ss << start.toString() << " -> " << end.toString();
    if (start.x == end.x) {
        ss << " (vertical ; x-intercept = " << start.x << ") ";
    } else {
        const int slope = (end.y - start.y) / (end.x - start.x);
        ss << " (slope = " << slope << "; ";
        ss << " y-intercept = " << start.y - slope * start.x << ") ";
    }
    return ss.str();
}

bool overlap_range(int a1, int a2, int b1, int b2, std::pair<int, int> &result) {
    if (a1 > a2) {
        std::swap(a1, a2);
    }
    if (b1 > b2) {
        std::swap(b1, b2);
    }

    int x1 = std::max(a1, b1);
    int x2 = std::min(a2, b2);

    if (x1 > x2) {
        return false;
    }

    result.first = x1;
    result.second = x2;
    return true;
}

int Line::signedArea2(const Point p) const {
    return (p.x - start.x) * (end.y - start.y) - (p.y - start.y) * (end.x - start.x);
}

bool Line::is_collinear(const Point p) const {
    return signedArea2(p) == 0;
}

bool Line::is_collinear(const Line other) const {
    return (signedArea2(other.start) == 0 && signedArea2(other.end) == 0);
}

Line::Line(std::string line) {
    std::string start_point_text, end_point_text;
    const std::string pt_delim = "->";
    auto location = line.find(pt_delim);
    
    start_point_text = line.substr(0, location);
    end_point_text = line.substr(location + pt_delim.length(), line.length());

    start = Point(start_point_text);
    end = Point(end_point_text);

    if (end < start) {
        std::swap(start, end);
    }
}

std::vector<Point> Line::get_points() const {
    std::vector<Point> points;
    if (start.x == end.x) {
        for (int y=start.y; y<=end.y; y++) {
            points.push_back(Point{start.x, y});
        }
    } else if (start.y == end.y) {
        for (int x=start.x; x<=end.x; x++) {
            points.push_back(Point{x, start.y});
        }
    } else {
        // 45 degree line
        const int ydir = (start.y < end.y) ? +1 : -1;
        for (int x=start.x; x<=end.x; x++) {
            points.push_back(Point{x, start.y + (x - start.x) * ydir});
        }
    }

    return points;
}

bool Line::contains_point(const Point p) const {
    if (p.x < start.x || p.x > end.x || p.y < std::min(start.y,end.y) || p.y > std::max(start.y,end.y)) {
        return false;
    }

    if ((start.x == end.x) && (p.x == start.x) && (p.y >= start.y && p.y <= end.y)) {
        // within a vertical line
        return true;
    }

    int slope = (end.y - start.y) / (end.x - start.x);  // must be +1, -1, or 0
    int dx = p.x - start.x;
    int expected_y = start.y + slope * dx;

    return (p.y == expected_y);
}

std::vector<Point> Line::intersection(const Line other) const {
    std::vector<Point> result = {};

    std::pair<int, int> x_range, y_range;
    // bounding boxes don't overlap - no possible interesection
    if (!overlap_range(start.x, end.x, other.start.x, other.end.x, x_range) || !overlap_range(start.y, end.y, other.start.y, other.end.y, y_range)) {
        return result;
    }

    if (is_grid_aligned() && other.is_grid_aligned()) {
        // if both grid aligned collinear, the overlapping bounding box defines the start/end point of the intersection segment
        Line segment(x_range.first, y_range.first, x_range.second, y_range.second);
        for (const Point &p : segment.get_points())
            result.push_back(p);
    } else if (is_collinear(other)) {
        // both diagonal with same slope
        // necessarily both +diag or -diag or else they'd both be grid algined
        const int slope = (end.y - start.y) / (end.x - start.x);
        int startx, starty;
        if (start.x > other.start.x) {
            startx = start.x;
            starty = start.y;
        } else {
            startx = other.start.x;
            starty = other.start.y;
        }

        for (int x=startx; x<=std::min(end.x, other.end.x); x++) {
            int y = starty + slope * (x - startx);
            result.push_back(Point{x, y});
        }
    } else if ((is_left(other.start) ^ is_left(other.end)) && (other.is_left(start) ^ other.is_left(end))) {
        Line l1 = *this;
        Line l2 = other;
        if (l1.start.x != l1.end.x) {
            std::swap(l1, l2);
        }

        // if one is vertical, it's the first one 
        int other_slope = (l2.end.y - l2.start.y) / (l2.end.x - l2.start.x);
        if (l1.start.x == l1.end.x) {
            int other_y = l2.start.y + (l1.start.x - l2.start.x) * other_slope;
            result.push_back(Point{l1.start.x, other_y});
        } else {
            // neither is vertical
            int slope = (l1.end.y - l1.start.y) / (l1.end.x - l1.start.x);
            if (other_slope == 0) {
                int y = l2.start.y;
                int x = (y-l1.start.y) / slope + l1.start.x;
                result.push_back(Point{x, y});
            } else {
                const int double_x = 2*(other_slope*l2.start.x - slope*l1.start.x - (l2.start.y - l1.start.y))/(other_slope-slope);
                if (double_x % 2 == 0) {
                    const int x = double_x / 2;
                    const int y = slope * (x - l1.start.x) + l1.start.y;
                    result.push_back(Point{x, y});
                }
            }
        }
    }

    std::vector<Point> final_result;
    for (const Point intersection_point : result) {
        if (!contains_point(intersection_point) || !other.contains_point(intersection_point)) {
            std::cout << "INVALID POINT! " << intersection_point.toString() << " is not on " << toString() << " and " << other.toString() << std::endl;
        } else {
            final_result.push_back(intersection_point);
        }
    }

    return final_result;
}

int main() {
    std::vector<Line> hv_lines;
    std::vector<Line> all_lines;

    while (std::cin) {
        std::string textline;
        std::getline(std::cin, textline);
        if (textline.empty())
            continue;

        Line newline = Line(textline);
        all_lines.push_back(newline);
        if (newline.is_grid_aligned()) {
            hv_lines.push_back(newline);
        }
    }


    std::cout << "Part 1: " << std::endl;
    std::set<Point> hv_intersection_points;
    for (int i=1; i<hv_lines.size(); i++) {
        for (int j=0; j<i; j++) {
            auto points = hv_lines[i].intersection(hv_lines[j]);
            for (auto point : points) {
                hv_intersection_points.insert(point);
            }
        }
    }
    std::cout << "  horiz/vert intersections: " << hv_intersection_points.size() << std::endl;

    int map[1000][1000];
    for (int i=0; i<1000; i++) {
        for (int j=0; j<1000; j++) {
            map[i][j] = 0;
        }
    }
    for (auto line : all_lines) {
        for (auto point : line.get_points()) {
            map[point.x][point.y] += 1;
        }
    }
    int count = 0;
    for (int i=0; i<1000; i++) {
        for (int j=0; j<1000; j++) {
            if (map[i][j] > 1)
                count++;
        }
    }

    std::cout << "Part 2: " << std::endl;
    std::set<Point> all_intersection_points;
    for (int i=1; i<all_lines.size(); i++) {
        for (int j=0; j<i; j++) {
            auto points = all_lines[i].intersection(all_lines[j]);
            for (auto point : points) {
                if (map[point.x][point.y] < 2) {
                    std::cout << "  Found wrong point " << point.toString() << std::endl;
                    std::cout << " at intersection of " << all_lines[i].toString() << " and " << all_lines[j].toString() << std::endl;
                    if (all_lines[i].contains_point(point) && all_lines[j].contains_point(point)) {
                        std::cout << "    and both lines contain it" << std::endl;
                    }
                }
                all_intersection_points.insert(point);
            }
        }
    }
    std::cout << "  all intersections: " << all_intersection_points.size() << std::endl;
    std::cout << "  all intersections on map = " << count << std::endl;
}