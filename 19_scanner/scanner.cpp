#include <array>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>

template <size_t N>
struct Coordinate {
    std::array<int, N> x;
    Coordinate(std::array<int, N> x) : x(x) {}
    Coordinate(int i=0) { for (int dim=0; dim<N; dim++) x[dim] = i; }
    Coordinate operator-(const Coordinate& other) const {
        std::array<int, N> result;
        for (size_t i=0; i<N; i++) {
            result[i] = x[i] - other.x[i];
        }
        return Coordinate(result);
    }
    int& operator[](int idx) { return x[idx]; }
    bool operator==(const Coordinate& other) const {
        for (size_t i=0; i<N; i++) {
            if (x[i] != other.x[i]) return false;
        }
        return true;
    }
    bool operator<(const Coordinate& other) const {
        for (size_t i=0; i<N; i++) {
            if (x[i] > other.x[i]) return false;
            if (x[i] < other.x[i]) return true;
        }
        return false;
    }
    std::string to_string() const { std::string s; for (int i=0; i<N-1; i++) s += std::to_string(x[i]) + ", "; s += std::to_string(x[N-1]); return s; }
};

template <size_t N>
struct CoordinateList {
    std::set<Coordinate<N>> x;
    size_t size() const { return x.size(); };
    size_t dims() const { return N; };
    CoordinateList() { x.clear(); };
    CoordinateList(const std::vector<std::array<int, N>>& coords) {
        for (const auto& coord : coords) 
            x.insert(Coordinate<N>(coord));
    }
    std::string to_string() const { std::string s; for (auto c : x) { s += c.to_string() + "\n"; } return s; }
    void add(const Coordinate<N>& c) { 
        x.insert(c);
    }
    bool has(const Coordinate<N>& c) const {
        return x.find(c) != x.end();
    }
    CoordinateList<N>& operator+=(const CoordinateList<N>& other) {
        for (const auto& coord : other.x) {
            add(coord);
        }
        return *this;
    }
};

template <size_t N>
struct CoordinateOrientation {
    std::array<int, N> coordinate_mapping;
    std::array<bool, N> coordinate_sign;
    CoordinateOrientation(const std::array<int, N>& map, const std::array<bool, N>& sign) : coordinate_mapping(map), coordinate_sign(sign) {};
    CoordinateOrientation(const std::array<int, N>& map) {
        for (int i=0; i<N; i++) {
            coordinate_sign[i] = true;
            coordinate_mapping[i] = map[i];
            if (map[i] < 0) {
                coordinate_mapping[i] = -coordinate_mapping[i];
                coordinate_sign[i] = false;
            }
            coordinate_mapping[i]--;
        }
    }
    CoordinateOrientation() {
        for (int i=0; i<N; i++) {
            coordinate_mapping[i] = i;
            coordinate_sign[i] = true;
        }
    }
    std::string to_string() const {
        std::string s;
        for (int i=0; i<N; i++) {
            s += coordinate_sign[i] ? "+" : "-";
            s += char('X'+coordinate_mapping[i]);
            s += " ";
        }
        return s;
    }
};

std::vector<CoordinateOrientation<2>> all_orientations(const std::array<int, 2>& dummy) {
    std::vector<std::vector<int>> transforms = {{1,2},{2,-1},{-2,-1},{-2,1}};
    std::vector<CoordinateOrientation<2>> orientations;

    for (auto t: transforms) {
        std::array<int, 2> ta = {t[0], t[1]};
        orientations.push_back(CoordinateOrientation<2>(ta));
    }
    return orientations;
}

std::vector<std::vector<int>> rotation_matrix(int ca, int cb, int cg, int sa, int sb, int sg) {
    std::vector<std::vector<int>> matrix(3, std::vector<int>(3, 0));
    matrix[0] = {ca*cb, ca*sb*sg - sa*cg, ca*sb*cg + sa*sg};
    matrix[1] = {sa*cb, sa*sb*sg + ca*cg, sa*sb*cg - ca*sg};
    matrix[2] = {-sb, cb*sg, cb*cg};
    return matrix;
}

std::array<int, 3> matrix_vector_mult(std::vector<std::vector<int>> matrix, std::array<int, 3> vector) {
    std::array<int, 3> result;
    for (int i=0; i<3; i++) {
        result[i] = 0;
        for (int j=0; j<3; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
    return result;
}

std::vector<CoordinateOrientation<3>> all_orientations(const std::array<int, 3>& dummy) {
    std::set<std::array<int, 3>> orientations;
    const std::vector<int> cos_theta =  {1, 0, -1, 0};
    const std::vector<int> sin_theta =  {0, 1,  0, -1};

    std::array<int, 3> mapping = {1,2,3};
    for (int a: {0,1,2,3}) {
        for (int b: {0,1,2,3}) {
            for (int g: {0,1,2,3}) {
                auto matrix = rotation_matrix(cos_theta[a], cos_theta[b], cos_theta[g], sin_theta[a], sin_theta[b], sin_theta[g]);
                auto result = matrix_vector_mult(matrix, mapping);
                orientations.insert(result);
            }
        }
    }
    std::vector<CoordinateOrientation<3>> result;
    for (auto& orientation : orientations) {
        result.push_back(CoordinateOrientation<3>(orientation));
    }

    return result;
}

template <size_t N>
struct CoordinateTransformation {
    CoordinateOrientation<N> orient;
    std::array<int, N> shift;
    CoordinateTransformation(const CoordinateOrientation<N>& orient, const std::array<int, N>& shift) : orient(orient), shift(shift) {};
    CoordinateTransformation(const CoordinateOrientation<N>& orient) : orient(orient) { for (int dim=0; dim<N; dim++) shift[dim] = 0; };
    CoordinateTransformation() : orient(), shift() {};
    std::string to_string() const {
        std::string s;
        s += orient.to_string() + "\n";
        for (int dim=0; dim<N; dim++) {
            s += std::to_string(shift[dim]) + " ";
        }
        return s;
    }
};

template<size_t N>
CoordinateList<N> applyTransformation(const CoordinateList<N>& x, const CoordinateTransformation<N>& trans) {
    CoordinateList<N> result;

    for (auto oldcoord: x.x) {
        Coordinate<N> newx(0);
        for (int dim=0; dim<N; dim++) {
            int alt_dim = trans.orient.coordinate_mapping[dim];
            int sign = trans.orient.coordinate_sign[dim] ? +1 : -1;
            newx[dim] = oldcoord[alt_dim] * sign + trans.shift[dim];
        }
        result.add(newx);
    }

    return result;
}

//
// Just brute-force it.  Absolutely awful.
//
template<size_t N>
int find_best_overlap(const CoordinateList<N>& c1, const CoordinateList<N>& c2, std::array<int, N>& best_offset) {
    std::map<std::array<int, N>, int> offset_to_count;
    for (auto x1: c1.x) {
        for (auto x2: c2.x) {
            Coordinate offset = x1 - x2;
            offset_to_count[offset.x]++;
        }
    }

    int best_overlap = 0;
    for (auto [offset, count] : offset_to_count) {
        if (count > best_overlap) {
            best_overlap = count;
            best_offset = offset;
        }
    }
    
    return best_overlap;
}

template<size_t N>
int find_best_transformation(const CoordinateList<N>& c1, const CoordinateList<N>& orig_c2, CoordinateTransformation<N>& best_transformation) {

    int best_overlap = 0;
    std::array<int, N> best_offset;
    CoordinateOrientation<N> best_orientation;

    const std::vector<CoordinateOrientation<N>> orientations = all_orientations(best_offset);

    for (const CoordinateOrientation<N>& orientation : orientations) {
        CoordinateList<N> c2 = applyTransformation<N>(orig_c2, CoordinateTransformation<N>(orientation));
        std::array<int, N> offset;

        int overlap = find_best_overlap(c1, c2, offset);
        if (overlap > best_overlap) {
            best_overlap = overlap;
            best_orientation = orientation;
            best_offset = offset;
        }
    }

    best_transformation = CoordinateTransformation<N>(best_orientation, best_offset);
    return best_overlap;
}

std::vector<CoordinateList<3>> get_inputs(std::ifstream& input) {
    std::vector<CoordinateList<3>> result;

    const std::regex delim_re("--- scanner (\\d+) ---");
    const std::regex coords_re("([-]?\\d+),([-]?\\d+),([-]?\\d+)");
    std::smatch smatches;
    bool first = true;

    std::string line;
    CoordinateList<3> x;

    while (std::getline(input, line)) {
        if (line.empty()) continue;

        if (std::regex_match(line, smatches, delim_re)) {
            if (first) {
                first = false;
                continue;
            } 
            result.push_back(x);
            x = CoordinateList<3>();
            continue;
        }

        if (std::regex_match(line, smatches, coords_re)) {
            Coordinate<3> coords({std::stoi(smatches[1].str()), std::stoi(smatches[2].str()), std::stoi(smatches[3].str())});
            x.add(coords);
            continue;
        }
    }

    if (x.size() != 0) {
        result.push_back(x);
    }
    return result;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    auto scanners = get_inputs(input);

    CoordinateList<3> transformed = scanners[0];
    scanners.erase(scanners.begin());

    std::vector<CoordinateTransformation<3>> transformations;
    while (!scanners.empty()) {
        std::vector<int> to_erase;
        for (int i=0; i<scanners.size(); i++) {
            CoordinateTransformation<3> trans;
            int number_aligned = find_best_transformation(transformed, scanners[i], trans);
            if (number_aligned >= 12) {
                to_erase.push_back(i);
                scanners[i] = applyTransformation(scanners[i], trans);
                transformations.push_back(trans);
            }
        }

        std::reverse(to_erase.begin(), to_erase.end());
        for (int i: to_erase) {
            transformed += scanners[i];
            scanners.erase(scanners.begin() + i);
        }
    }

    std::cout << "Part 1:" << std::endl;
    std::cout << "      " << transformed.size() << std::endl;

    transformations.push_back(CoordinateTransformation<3>());
    int max_dist = 0;
    for (auto trans1: transformations) {
        auto offset1 = Coordinate<3>(trans1.shift);
        for (auto trans2: transformations) {
            auto offset2 = Coordinate<3>(trans2.shift);

            auto dist = offset1 - offset2;
            int dist_mag = 0;
            for (int i=0; i<3; i++) 
                dist_mag += abs(dist[i]);

            if (dist_mag > max_dist)
                max_dist = dist_mag;
        }
    }
    std::cout << "Part 2:" << std::endl;
    std::cout << "      " << max_dist << std::endl;

}