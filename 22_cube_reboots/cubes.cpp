#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <string>
#include <regex>

struct Range {
    int start;
    int end;
    Range() : start(0), end(0) {}
    Range(const int x1, const int x2) : start(x1), end(x2) {}
    bool empty() const {
        return start > end;
    }
    bool operator==(const Range& other) const {
        return start == other.start && end == other.end;
    }
    bool operator<(const Range& other) const {
        if (start == other.start) {
            return end < other.end;
        } else {
            return start < other.start;
        }
    }
    int operator[] (int i) const {
        if (i == 0)
            return start;
        if (i == 1)
            return end;
        throw std::out_of_range("Range::operator[]");
    }
    unsigned long int size() const {
        if (start <= end) {
            return end - start + 1;
        } else {
            return 0ul;
        }
    }
    Range overlap(const Range& other) const {
        int new_start = std::max(start, other.start);
        int new_end = std::min(end, other.end);
        return Range(new_start, new_end); 
    }
    std::string to_string() const {
        return "[" + std::to_string(start) + "," + std::to_string(end) + "]";
    }
};

class Block {
    public:
        Block(int x1, int x2, int y1, int y2, int z1, int z2, bool state) : state(state) {
            coords[0] = Range(x1, x2);
            coords[1] = Range(y1, y2);
            coords[2] = Range(z1, z2);
        };
        Block(const Block& other) {
            state = other.state;
            for (int i=0; i<3; i++) {
                coords[i] = other.coords[i];
            }
        }
        Block() {
            state = false;
            for (int i=0; i<3; i++) {
                coords[i] = Range(0, -1);
            }
        }
        bool operator==(const Block& other) const {
            for (int dim=0; dim<3; dim++)
                if (!(coords[dim] == other.coords[dim]))
                    return false;
            return true;
        }
        unsigned long int volume() const {
            unsigned long int vol = 1ul;
            for (int dim=0; dim<3; dim++)
                vol *= coords[dim].size();
            return vol;
        };
        Block overlap(const Block& other) const {
            Block new_block(0, -1, 0, -1, 0, -1, state);
            for (int dim=0; dim<3; dim++) {
                auto r = coords[dim].overlap(other.coords[dim]);
                if (r.empty()) {
                    return new_block;
                }
                new_block.coords[dim] = r;
            }
            return new_block;
        };
        bool empty() const {
            return (volume() == 0);
        }
        bool is_on() const {
            return state;
        }
        std::string to_string() const {
            std::string s = "";
            for (int dim=0; dim<2; dim++) {
                s += coords[dim].to_string();
                s += " x ";
            }
            s += coords[2].to_string();
            s += state ? " ON" : " OFF";
            return s;
        }
        // returns a list of blocks contained by this block but excluding intersection with the other
        std::vector<Block> remove_block(const Block& other) const {
            std::vector<Block> result;
            if (empty()) 
                return result;

            Block intersection = overlap(other);
            if (intersection.empty()) {
                result.push_back(Block(*this));
                return result;
            }

            // create all the relevant ranges
            std::set<Range> new_ranges[3];
            for (int dim=0; dim<3; dim++) {
                auto r = Range(coords[dim][0], intersection.coords[dim][0]-1);
                if (!r.empty())
                    new_ranges[dim].insert(r);

                r = Range(intersection.coords[dim][0], intersection.coords[dim][1]);
                if (!r.empty())
                    new_ranges[dim].insert(r);

                r = Range(intersection.coords[dim][1]+1, coords[dim][1]);
                if (!r.empty())
                    new_ranges[dim].insert(r);
            }

            // create the new blocks
            for (const auto&rx: new_ranges[0]) {
                for (const auto&ry: new_ranges[1]) {
                    for (const auto&rz: new_ranges[2]) {
                        Block new_block(rx[0], rx[1], ry[0], ry[1], rz[0], rz[1], state);
                        if (!new_block.empty() && !(new_block == intersection)) {
                            result.push_back(new_block);
                        }
                    }
                }
            }

            return result;
        }

    private:
        Range coords[3];
        bool state;
};

class Reactor {
    public:
        Reactor() : clipping(false) {}
        Reactor(const int x1, const int x2, const int y1, const int y2, const int z1, const int z2) : clipping(true) {
            clip_region = Block(x1, x2, y1, y2, z1, z2, true);
        }
        unsigned long int volume_on() const {
            unsigned long int vol = 0ul;
            for (const auto& b : on_regions) {
                vol += b.volume();
            }
            return vol;
        }
        void insert(const Block& b);
        void erase(const Block& b);
        //
    private:
        std::vector<Block> on_regions;
        bool clipping;
        Block clip_region;
};

void Reactor::insert(const Block& b) {
    auto clipped_block = b;

    if (clipping) {
        clipped_block = b.overlap(clip_region);

    if (clipped_block.empty())
        return;
    }

    std::vector<Block> blocks_to_add;
    blocks_to_add.push_back(clipped_block);

    while (!blocks_to_add.empty()) {
        Block block_to_add = blocks_to_add.back();
        blocks_to_add.pop_back();

        bool found_overlap = false;
        for (auto& on_region : on_regions) {
            Block overlap = on_region.overlap(block_to_add);
            if (!overlap.empty()) {
                // there's non-zero overlap between this new block and the on regions
                // so we need to split this block into smaller blocks and add them to the on regions
                auto new_blocks = block_to_add.remove_block(overlap);
                blocks_to_add.insert(blocks_to_add.end(), new_blocks.begin(), new_blocks.end());
                found_overlap = true;
                break;
            }
        }
        // no overlaps; can safely add
        if (!found_overlap)
            on_regions.push_back(block_to_add);
    }
}

void Reactor::erase(const Block& b) {
    auto clipped_block = b;

    if (clipping) {
        clipped_block = b.overlap(clip_region);

    if (clipped_block.empty())
        return;
    }

    std::vector<Block> blocks_to_erase;
    for (const auto& on_region : on_regions) {
        Block overlap = on_region.overlap(clipped_block);
        if (!overlap.empty()) {
            // there's non-zero overlap between this new off block and the on regions
            // so we need to split the on region into smaller blocks and remove the overlap from the on regions
            auto new_blocks = on_region.remove_block(overlap);

            blocks_to_erase.push_back(on_region);
            for (auto& new_block : new_blocks) {
                on_regions.push_back(new_block);
            }
        }
        // no overlap - nothing to turn off.  no action taken
    }

    for (const auto& block_to_erase : blocks_to_erase) {
        auto it = std::find(on_regions.begin(), on_regions.end(), block_to_erase);
        on_regions.erase(it);
    }
}

std::vector<Block> get_inputs(std::ifstream &input) {
    std::vector<Block> result;
    std::string line;
    std::regex pattern_re("(on|off) x=([+-]?\\d+)..([+-]?\\d+),y=([+-]?\\d+)..([+-]?\\d+),z=([+-]?\\d+)..([+-]?\\d+)");

    while (std::getline(input, line)) {
        std::smatch match;
        if (std::regex_match(line, match, pattern_re)) {
            bool state = (match[1] == "on");
            int x1 = std::stoi(match[2]);
            int x2 = std::stoi(match[3]);
            int y1 = std::stoi(match[4]);
            int y2 = std::stoi(match[5]);
            int z1 = std::stoi(match[6]);
            int z2 = std::stoi(match[7]);
            result.push_back(Block(x1, x2, y1, y2, z1, z2, state));
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
    if (!input.is_open()) {
        std::cerr << "Could not open input file " << argv[1] << std::endl;
        return 2;
    }

    std::vector<Block> blocks = get_inputs(input);
    std::vector<Reactor> reactors = {Reactor(-50, +50, -50, +50, -50, +50), Reactor()};

    for (int i=0; i<reactors.size(); i++) {
        std::cout << "Part " << i << ": " << std::endl;
        for (const auto& block : blocks) {
            if (block.is_on()) {
                reactors[i].insert(block);
            } else {
                reactors[i].erase(block);
            }
        }

        std::cout << "      : " << reactors[i].volume_on() << std::endl;
        std::cout << std::endl;
    }
}