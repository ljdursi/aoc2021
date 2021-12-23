#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

const short int NROOMS=4;
const short int NSPOTS_PER_ROOM=2;
const short int NHALLWAY=(2*NROOMS-1)+4;

enum struct AmphipodType {
    AMBER, BRONZE, COPPER, DESERT, NONE
};

const std::map<AmphipodType, short int> type_to_room {
    {AmphipodType::AMBER, 0}, {AmphipodType::BRONZE, 1}, {AmphipodType::COPPER, 2}, {AmphipodType::DESERT, 3}
}; 

const std::map<AmphipodType, unsigned short int> type_to_energy {
    {AmphipodType::AMBER, 1}, {AmphipodType::BRONZE, 10}, {AmphipodType::COPPER, 100}, {AmphipodType::DESERT, 1000} }; 

const std::map<AmphipodType, char> type_to_char = {{AmphipodType::AMBER, 'A'}, {AmphipodType::BRONZE, 'B'}, {AmphipodType::COPPER, 'C'}, {AmphipodType::DESERT, 'D'}, {AmphipodType::NONE, '.'}};

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
    if (type == other.type) {
        if (type == LocationType::HALLWAY) {
            return index < other.index;
        } else {
            if (index < other.index)
                return true;
            return (spot < other.spot);
        }
    } else {
        return type < other.type;
    }
}

typedef std::map<Location, AmphipodType> AmphipodConfiguration;

std::string configuration_to_string(const AmphipodConfiguration& config) {
    std::stringstream ss;

    char hallway[NHALLWAY];
    char rooms[NROOMS][NSPOTS_PER_ROOM];

    for (int i=0; i<NHALLWAY; i++) 
        hallway[i] = '.';

    for (int i=0; i<NROOMS; i++)
        for (int j=0; j<NSPOTS_PER_ROOM; j++)
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

    for (short int spot=0; spot < NSPOTS_PER_ROOM; spot++) {
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

bool need_to_move(const AmphipodConfiguration& config, const Location& location, const AmphipodType& type) {
    if (location.type != LocationType::ROOM)
        return true;
    if (location.index != type_to_room.at(type))
        return true;

    // We're in the right room.
    // Are there other occupants below me in the room that I need to get out of the way of?
    short int spot = location.spot;
    for (short int i=spot+1; i<NSPOTS_PER_ROOM; i++) {
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
    for (short int i=location.spot-1; i>=0; i--) {
        auto it = config.find({LocationType::ROOM, location.index, i});
        if (it != config.end())
        return false;
    }
    return true;
}

bool solved(const AmphipodConfiguration& config) {
    for (const auto& [location, type]: config)
        if (need_to_move(config, location, type))
            return false;

    return true;
}

std::vector<Location> rooms_available(const AmphipodConfiguration& config, const AmphipodType amph_type) {
    if (amph_type == AmphipodType::NONE) {
        return std::vector<Location>();
    }

    short int needed_room = type_to_room.at(amph_type);
    short int n_occupiers = 0;
    bool occupied_by_other_type = false;

    for (const auto& [location, type]: config) {
        if ((location.type == LocationType::ROOM) && (location.index == needed_room)) {
            n_occupiers++;
            if (type_to_room.at(type) != needed_room) {
                occupied_by_other_type = true;
            }
        }
    }

    std::vector<Location> result;
    if (occupied_by_other_type)
        return result;

    if (n_occupiers < NSPOTS_PER_ROOM) {
        result.push_back(Location{LocationType::ROOM, needed_room, (short int)(NSPOTS_PER_ROOM-n_occupiers-1)});
    }
    return result;
}

bool hallway_path_blocked(const AmphipodConfiguration& config, const Location& from, const Location& to) {
    short int start_hallway = (from.type == LocationType::HALLWAY) ? from.index : 2*from.index + 2;
    short int to_hallway = (to.type == LocationType::HALLWAY) ? to.index : 2*to.index + 2;

    start_hallway = (to_hallway > start_hallway) ? start_hallway + 1 : start_hallway - 1;

    for (const auto& [location, type]: config) {
        if (location.type != LocationType::HALLWAY) 
            continue;
        if ((location.index >= std::min(start_hallway, to_hallway)) && (location.index <= std::max(start_hallway, to_hallway)))
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
        if (!hallway_path_blocked(config, at, {LocationType::HALLWAY, i})) {
            result.push_back(Location{LocationType::HALLWAY, i, 0});
        }
    }

    return result;
}

struct Transition {
    Location from, to;
    AmphipodType amph_type;
};

std::vector<Transition> legal_moves(const AmphipodConfiguration& configuration) {
    std::vector<Transition> result;

    std::map<AmphipodType, std::vector<Location>> available_rooms;
    for (AmphipodType type: {AmphipodType::AMBER, AmphipodType::BRONZE, AmphipodType::COPPER, AmphipodType::DESERT}) {
        available_rooms[type] = rooms_available(configuration, type);
    }
    
    for (const auto& [location, inhabitant]: configuration) {
        switch (location.type) {
            case LocationType::HALLWAY :
                // from the hallway, the only valid move is to a final room
                for (const auto& to: available_rooms[inhabitant] ) {
                    if (!hallway_path_blocked(configuration, location, to)) {
                        result.push_back({location, to, inhabitant});
                    }
                }
                break;
            case LocationType::ROOM :
                // from a room, the only valid move is to a hallway or to the final room
                // Do I not need to move?
                if (!need_to_move(configuration, location, inhabitant))
                    continue;

                // Am I blocked in anyway?
                if (!top_of_room(configuration, location))
                    continue;

                for (const auto& to: available_rooms[inhabitant] ) {
                    if (!hallway_path_blocked(configuration, location, to)) {
                        result.push_back({location, to, inhabitant});
                    }
                }
                for (const auto& to: hallway_positions_available(configuration, location) ) {
                    result.push_back({location, to, inhabitant});
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
    return distance(transition.from, transition.to) * type_to_energy.at(transition.amph_type);
}

void update_config(AmphipodConfiguration& config, const Transition& transition) {
    config.erase(transition.from);
    config[transition.to] = transition.amph_type;
}

long int minimum_energy_path(const AmphipodConfiguration& config, std::vector<Transition>& path, const long int best_so_far=INT_MAX) {
    long int min_energy = best_so_far; 
    Transition best_transition;
    std::vector<Transition> best_path;
    AmphipodConfiguration current = config;

    if (solved(config)) {
        std::cout << "SOLVED!!!" << std::endl;
        return 0;
    }

    std::vector<Transition> transitions = legal_moves(config);

    int ntransitions = transitions.size(), count = 0;
    for (const auto& transition : transitions) {
        std::cout << "Path depth " << path.size() << " trying " << 1+(count++) << "/" << ntransitions << std::endl;

        long int trial_energy = transition_energy(transition);
        // don't even bother if we've already spent our energy budet
        if (trial_energy > min_energy) {
            continue;
        }

        update_config(current, transition);
        std::cout << configuration_to_string(current) << std::endl;

        std::vector<Transition> subpath(path);
        subpath.push_back(transition);

        trial_energy += minimum_energy_path(current, subpath, min_energy);
        if (trial_energy < min_energy) {
            min_energy = trial_energy;
            best_path = subpath;
            best_transition = transition;
        }

        subpath.pop_back();
        update_config(current, Transition{transition.to, transition.from, transition.amph_type});
    }

    path.insert(path.end(), best_path.begin(), best_path.end());

    return min_energy;
}


int main() {
    AmphipodConfiguration ac;

    ac[Location{LocationType::ROOM, 0, 0}] = AmphipodType::BRONZE;
    ac[Location{LocationType::ROOM, 0, 1}] = AmphipodType::AMBER;

    ac[Location{LocationType::ROOM, 1, 0}] = AmphipodType::COPPER;
    ac[Location{LocationType::ROOM, 1, 1}] = AmphipodType::DESERT;

    ac[Location{LocationType::ROOM, 2, 0}] = AmphipodType::BRONZE;
    ac[Location{LocationType::ROOM, 2, 1}] = AmphipodType::COPPER;

    ac[Location{LocationType::ROOM, 3, 0}] = AmphipodType::DESERT;
    ac[Location{LocationType::ROOM, 3, 1}] = AmphipodType::AMBER;

    std::cout << configuration_to_string(ac) << std::endl;

    std::vector<Transition> path;
    unsigned int energy = minimum_energy_path(ac, path);

    std::cout << "Energy: " << energy << std::endl;
}