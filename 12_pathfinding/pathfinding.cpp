#include <vector>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

struct Cave {
    std::string name;
    int idx;
    bool small;
};

class CaveGraph {
    public:  
        CaveGraph() {};
        bool add_vertex(std::string name);
        bool add_edge(std::string start, std::string end);
        std::string to_string();
        std::vector<std::string> get_paths(std::string start, std::string end, int n_small_visits);
    private:
        std::vector<Cave> caves;
        std::vector<std::vector<int>> adjacency_list;
        std::map<std::string, int> cave_idx;
        std::string path_to_string(std::vector<int> path);
};

bool CaveGraph::add_vertex(std::string name) {
    if (cave_idx.find(name) != cave_idx.end()) {
        return false;
    }

    bool small = true;
    for (const char c: name) {
        if (!islower(c)) {
            small = false;
            break;
        }
    }
    int idx = caves.size();
    caves.push_back({name, idx, small});
    cave_idx[name] = idx;

    adjacency_list.push_back({});
    return true;
}

bool CaveGraph::add_edge(std::string start, std::string end) {
    if (cave_idx.find(start) == cave_idx.end() || cave_idx.find(end) == cave_idx.end()) {
        return false;
    }

    int start_idx = cave_idx.at(start);
    int end_idx = cave_idx.at(end);

    if (std::find(adjacency_list[start_idx].begin(), adjacency_list[start_idx].end(), end_idx) != adjacency_list[start_idx].end()) {
       return false;
    }
    adjacency_list[start_idx].push_back(end_idx);

    if (std::find(adjacency_list[end_idx].begin(), adjacency_list[end_idx].end(), start_idx) != adjacency_list[end_idx].end()) {
       return false;
    }
    adjacency_list[end_idx].push_back(start_idx);

    return true;
}

std::string CaveGraph::to_string() {
    std::stringstream ss;

    for (int i=0; i<caves.size(); i++) {
        std::string name = caves[i].name;

        for (int j: adjacency_list[i]) {
            ss << name << " -> " << caves[j].name << std::endl;
        }
    }
    return ss.str();
}

std::string CaveGraph::path_to_string(std::vector<int> path) {
    std::string delim=",";
    std::stringstream ss;
    bool first = true;

    for (auto i: path) {
        if (!first)
            ss << delim;
        ss << caves[i].name;
        first = false;
    }
    return ss.str();
}

std::vector<std::string> CaveGraph::get_paths(std::string start, std::string end, int n_small_visits = 0) {
    if (cave_idx.find(start) == cave_idx.end() || cave_idx.find(end) == cave_idx.end()) {
        return {{}};
    }

    int start_idx = cave_idx.at(start);
    int end_idx = cave_idx.at(end);

    std::vector<std::vector<int>> valid_paths;
    std::queue<std::pair<std::vector<int>, int>> path_queue;

    path_queue.push(std::make_pair<std::vector<int>,int>({start_idx}, 0));
    while (!path_queue.empty()) {
        auto current = path_queue.front();
        auto path = current.first;
        auto small_revisits_so_far = current.second;
        path_queue.pop();

        auto last = path.back();
        if (last == end_idx) {
            valid_paths.push_back(path);
            continue;
        } 

        for (int next: adjacency_list[last]) {
            if (caves[next].name == start) // can't visit start twice
                continue;

            bool already_visited = (std::find(path.begin(), path.end(), next) != path.end());
            bool is_small = caves[next].small;
            int new_small_revisits = (already_visited && is_small) ? 1 : 0;

            if (small_revisits_so_far + new_small_revisits > n_small_visits)
                continue;

            auto new_path = path;
            new_path.push_back(next);
            path_queue.push(std::make_pair(new_path, small_revisits_so_far + new_small_revisits));
        }
    }

    // Now generate the string representation of each path
    std::vector<std::string> path_representations;
    for (auto path: valid_paths) {
        path_representations.push_back(path_to_string(path));
    }

    return path_representations;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    CaveGraph cave_graph;

    std::ifstream input(argv[1]);    
    std::string line;
    const std::string delimiter = "-";
    while (std::getline(input, line)) {
        std::string start = line.substr(0, line.find(delimiter));
        std::string end = line.substr(line.find(delimiter)+1, line.length());

        cave_graph.add_vertex(start);
        cave_graph.add_vertex(end);
        cave_graph.add_edge(start, end);
    }

    std::vector<std::string> paths = cave_graph.get_paths("start", "end");
    std::cout << "Part 1:" << std::endl;
    std::cout << "      Number of paths: " << paths.size() << std::endl;

    std::vector<std::string> paths_w_repeats = cave_graph.get_paths("start", "end", 1);
    std::cout << "Part 2:" << std::endl;
    std::cout << "      Number of paths: " << paths_w_repeats.size() << std::endl;
}