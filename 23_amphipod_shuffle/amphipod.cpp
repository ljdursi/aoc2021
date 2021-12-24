#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <regex>
#include <fstream>
#include <climits>

const short int NROOMS=4;
const short int NHALLWAY=(2*NROOMS-1)+4;

enum struct AmphipodType {
    AMBER, BRONZE, COPPER, DESERT, NONE
};

const std::map<AmphipodType, short int> type_to_room {
    {AmphipodType::AMBER, 0}, {AmphipodType::BRONZE, 1}, {AmphipodType::COPPER, 2}, {AmphipodType::DESERT, 3}
}; 

enum struct LocationType {
    HALLWAY, ROOM
};

struct Location {
    LocationType type;
    short int index;
    short int spot=0;
    bool operator<(const Location& other) const;
};

bool Location::operator<(const Location& other) const {
    if (type != other.type)
        return type < other.type;

    if (index == other.index)
        return (spot < other.spot);
    else
        return index < other.index;
}

struct Transition {
    Location from, to;
    AmphipodType amph_type;
};

std::string transition_to_string(const Transition& t) {
    auto type_str = [](const LocationType type) { return (type == LocationType::HALLWAY) ? std::string("HALLWAY") : std::string("ROOM"); };
    auto loc_str = [](const Location loc) { return std::to_string(loc.index) + (loc.type == LocationType::ROOM ? "," + std::to_string(loc.spot) : ""); };

    return "( " + type_str(t.from.type) + " " + loc_str(t.from) + ") -> (" + type_str(t.to.type) + " " + loc_str(t.to) + " )";
}

typedef std::map<Location, AmphipodType> AmphipodConfiguration;

std::string configuration_to_string(const AmphipodConfiguration& config, const short int spots_per_room) {
    const std::map<AmphipodType, char> type_to_char = {{AmphipodType::AMBER, 'A'}, {AmphipodType::BRONZE, 'B'}, {AmphipodType::COPPER, 'C'}, {AmphipodType::DESERT, 'D'}, {AmphipodType::NONE, '.'}};
    std::stringstream ss;

    char hallway[NHALLWAY];
    char rooms[NROOMS][spots_per_room];

    for (int i=0; i<NHALLWAY; i++) 
        hallway[i] = '.';

    for (int i=0; i<NROOMS; i++)
        for (int j=0; j<spots_per_room; j++)
            rooms[i][j] = '.';

    for (const auto& [location, type]: config) {
        if (location.type == LocationType::HALLWAY) {
            hallway[location.index] = type_to_char.at(type);
        } else {
            rooms[location.index][location.spot] = type_to_char.at(type);
        }
    }

    for (int i=0; i<NHALLWAY+2; i++)
        ss << "#";
    ss << std::endl;

    ss << "#";
    for (short int i=0; i<NHALLWAY; i++) {
        ss << hallway[i];
    }
    ss << "#" << std::endl;

    for (short int spot=0; spot < spots_per_room; spot++) {
        ss << (spot == 0 ? "##" : "  ");
        for (short int room=0; room<NROOMS; room++) {
            ss << "#" << rooms[room][spot];
        }
        ss << (spot == 0 ? "###" : "#  ") << std::endl;
    }
    ss << "  ";
    for (int i=0; i<2*NROOMS+1; i++) 
        ss << "#";
    ss << std::endl;

    return ss.str();
}

bool need_to_move(const AmphipodConfiguration& config, const Location& location, const AmphipodType& type, const short int spots_per_room) {
    if (location.type != LocationType::ROOM)
        return true;
    if (location.index != type_to_room.at(type))
        return true;

    // We're in the right room.
    // Are there other occupants below me in the room that I need to get out of the way of?
    short int spot = location.spot;
    for (short int i=spot+1; i<spots_per_room; i++) {
        auto it = config.find({LocationType::ROOM, location.index, i});
        if (it != config.end() && it->second != type)
            return true;
    }

    // nope, we're good
    return false;
}

bool top_of_room(const AmphipodConfiguration& config, const Location& location) {
    if (location.type != LocationType::ROOM)
        return false;

    if (location.spot == 0)
        return true;

    // Am I blocked in?
    for (short int i=0; i<location.spot; i++) {
        auto it = config.find({LocationType::ROOM, location.index, i});
        if (it != config.end())
            return false;
    }
    return true;
}

bool solved(const AmphipodConfiguration& config, const short int spots_per_room) {
    for (const auto& [location, type]: config) {
        if (location.type != LocationType::ROOM)
            return false;
        if (location.index != type_to_room.at(type))
            return false;
    }
    return true;
}

std::vector<Location> rooms_available(const AmphipodConfiguration& config, const AmphipodType amph_type, const short int spots_per_room) {
    std::vector<Location> result;

    short int needed_room = type_to_room.at(amph_type);
    short int min_occupied = spots_per_room;
    short int n_occupiers = 0;

    for (short int slot=0; slot<spots_per_room; slot++) {
        auto it = config.find({LocationType::ROOM, needed_room, slot});
        if (it != config.end()) {
            min_occupied = std::min(min_occupied, slot);
            if (it->second == amph_type) {
                n_occupiers++;
            } else {
                // occupied by others type - bail
                return result;
            }
        } 
    }

    if (min_occupied > 0) 
        result.push_back(Location{LocationType::ROOM, needed_room, (short int)(min_occupied-1)});

    return result;
}

bool hallway_path_blocked(const AmphipodConfiguration& config, const Location& from, const Location& to) {
    short int start_hallway = (from.type == LocationType::HALLWAY) ? from.index : 2*from.index + 2;
    short int to_hallway = (to.type == LocationType::HALLWAY) ? to.index : 2*to.index + 2;

    start_hallway = (to_hallway > start_hallway) ? start_hallway + 1 : start_hallway - 1;

    for (short int i=std::min(start_hallway, to_hallway); i<=std::max(start_hallway, to_hallway); i++) {
        if (config.find({LocationType::HALLWAY, i, 0}) != config.end())
            return true;
    }
    return false;
}

std::vector<Location> hallway_positions_available(const AmphipodConfiguration &config, const Location &at) {
    std::vector<Location> result;

    auto blocks_room = [](const short int hallway_posn) { return ((hallway_posn % 2 == 0) && (hallway_posn >= 2) && (hallway_posn <= 2*NROOMS)); };

    for (short int i=0; i<11; i++) {
        if (blocks_room(i)) {
            continue;
        }
        Location dest = Location{LocationType::HALLWAY, i, 0};
        if (!hallway_path_blocked(config, at, dest)) {
            result.push_back(dest);
        }
    }

    return result;
}

std::vector<Transition> legal_moves(const AmphipodConfiguration& configuration, const short int spots_per_room) {
    std::vector<Transition> result;

    std::map<AmphipodType, std::vector<Location>> available_rooms;
    for (AmphipodType type: {AmphipodType::AMBER, AmphipodType::BRONZE, AmphipodType::COPPER, AmphipodType::DESERT}) {
        available_rooms[type] = rooms_available(configuration, type, spots_per_room);
    }
    
    for (const auto& [location, inhabitant]: configuration) {
        switch (location.type) {
            case LocationType::HALLWAY :
                // from the hallway, the only valid move is to a final room
                for (const auto& to: available_rooms[inhabitant] ) {
                    if (!hallway_path_blocked(configuration, location, to)) {
                        result.push_back(Transition{location, to, inhabitant});
                    }
                }
                break;
            case LocationType::ROOM :
                // from a room, the only valid move is to a hallway or to the final room
                // Am I blocked in anyway?
                if (!top_of_room(configuration, location)) 
                    continue;

                // Do I not need to move?
                if (!need_to_move(configuration, location, inhabitant, spots_per_room)) 
                    continue;

                for (const auto& to: available_rooms[inhabitant] ) {
                    if (!hallway_path_blocked(configuration, location, to)) {
                        result.push_back(Transition{location, to, inhabitant});
                    }
                }
                for (const auto& to: hallway_positions_available(configuration, location) ) {
                    result.push_back(Transition{location, to, inhabitant});
                }
                break;
        }
    }

    return result;
}

short int distance(const Location &from, const Location &to) {
    // start with number of hallway steps
    short int start_hallway = (from.type == LocationType::HALLWAY) ? from.index : 2*from.index + 2;
    short int to_hallway = (to.type == LocationType::HALLWAY) ? to.index : 2*to.index + 2;

    short int nsteps = std::abs(to_hallway - start_hallway);

    if (from.type == LocationType::ROOM) {
        nsteps += 1 + from.spot;
    }
    if (to.type == LocationType::ROOM) {
        nsteps += 1 + to.spot;
    }

    return nsteps;
}

int transition_energy(const Transition &transition) {
    const std::map<AmphipodType, unsigned short int> type_to_energy {
    {AmphipodType::AMBER, 1}, {AmphipodType::BRONZE, 10}, {AmphipodType::COPPER, 100}, {AmphipodType::DESERT, 1000} }; 

    return distance(transition.from, transition.to) * type_to_energy.at(transition.amph_type);
}

void update_config(AmphipodConfiguration& config, const Transition& transition) {
    config.erase(transition.from);
    config[transition.to] = transition.amph_type;
}

long int minimum_energy_path(const AmphipodConfiguration& config, const short int spots_per_room, const long int best_so_far=INT_MAX) {
    if (solved(config, spots_per_room))
        return 0;

    long int min_energy = best_so_far; 
    std::vector<Transition> transitions = legal_moves(config, spots_per_room);

    for (const auto& transition : transitions) {
        long int trial_energy = transition_energy(transition);
        // don't even bother if we've already spent our energy budet
        if (trial_energy > min_energy) {
            continue;
        }

        AmphipodConfiguration current(config);
        update_config(current, transition);

        trial_energy += minimum_energy_path(current, spots_per_room, min_energy-trial_energy);
        if (trial_energy < min_energy) {
            min_energy = trial_energy;
        }
    }
    return min_energy;
}

AmphipodConfiguration get_input(std::ifstream& input) {
    AmphipodConfiguration config;

    std::regex target("[ #]*(A|B|C|D)#(A|B|C|D)#(A|B|C|D)#(A|B|C|D)[# ]*");
    std::smatch match;
    std::string line;

    auto char_to_type = [](char c) { return (c == 'A') ? AmphipodType::AMBER : (c == 'B') ? AmphipodType::BRONZE : (c == 'C') ? AmphipodType::COPPER : AmphipodType::DESERT; };
    short int slot = 0;
    while (std::getline(input, line)) {
        if (std::regex_match(line, match, target)) {
            for (short int room=0; room<4; room++) {
                config[{LocationType::ROOM, room, slot}] = char_to_type(match[room+1].str()[0]);
            }
            slot++;
        }
    }

    return config;
}

AmphipodConfiguration unfold(const AmphipodConfiguration& config) {
    AmphipodConfiguration unfolded(config);

    const auto ROOMTYPE = LocationType::ROOM;

    for (const auto& [location, type]: config) {
        if (location.type == ROOMTYPE) {
            if (location.spot == 1) {
                for (short int newspot=2; newspot<4; newspot++) {
                    unfolded[{ROOMTYPE, location.index, newspot}] = type;
                }
            }
        }
    }

    //  #D#C#B#A#
    //  #D#B#A#C#

    unfolded[{ROOMTYPE, 0, 1}] = AmphipodType::DESERT;
    unfolded[{ROOMTYPE, 0, 2}] = AmphipodType::DESERT;

    unfolded[{ROOMTYPE, 1, 1}] = AmphipodType::COPPER;
    unfolded[{ROOMTYPE, 1, 2}] = AmphipodType::BRONZE;

    unfolded[{ROOMTYPE, 2, 1}] = AmphipodType::BRONZE;
    unfolded[{ROOMTYPE, 2, 2}] = AmphipodType::AMBER;

    unfolded[{ROOMTYPE, 3, 1}] = AmphipodType::AMBER;
    unfolded[{ROOMTYPE, 3, 2}] = AmphipodType::COPPER;

    return unfolded;
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

    auto ac = get_input(input);
    std::cout << configuration_to_string(ac, 2) << std::endl;
    unsigned int energy = minimum_energy_path(ac, 2);

    std::cout << "Part 1: " << std::endl;
    std::cout << "     Energy: " << energy << std::endl << std::endl;

    std::cout << "Part 2: " << std::endl;
    auto ac_unfolded = unfold(ac);
    std::cout << configuration_to_string(ac_unfolded, 4) << std::endl;
    energy = minimum_energy_path(ac_unfolded, 4);
    std::cout << "     Energy: " << energy << std::endl << std::endl;
}