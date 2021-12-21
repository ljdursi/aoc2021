#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

const int nplayers = 2;

struct UniverseState {
    short int positions[nplayers];
    short int scores[nplayers];

    // need this comparison operator so we can have a map of them
    bool operator<(const UniverseState& other) const {
        for (int i=0; i<nplayers; i++) {
            if (positions[i] < other.positions[i])
                return true;
            if (scores[i] < other.scores[i])
                return true;
            if (positions[i] > other.positions[i])
                return false;
            if (scores[i] > other.scores[i])
                return false;
        }
        return false;
    }
};

class MultiverseSimulator {
    public:
        static const int board_size = 10;
        static const int max_score = 21;

        MultiverseSimulator(const std::vector<long int>& die_rolls, const std::vector<short int>& starting_positions) : die_roll_splits(die_rolls) {
            UniverseState initial_state;
            for (int i=0; i<nplayers; i++) {
                initial_state.positions[i] = starting_positions[i];
                initial_state.scores[i] = 0;
                nwins[i] = 0l;
            }
            Universes[initial_state] = 1l;
            nuniverses_playing = 1l;
        }

        void turn() {
            std::map<UniverseState, long int> new_universes;
            long int still_playing = 0;
            for (auto& [universe, count] : Universes) {
                for (short int move=0; move<die_roll_splits.size(); move++) {
                    long int n_outcomes = die_roll_splits[move];
                    if (n_outcomes == 0)
                        continue;

                    long int nuniverses = count * n_outcomes;

                    UniverseState new_state = universe;
                    new_state.positions[player_turn] = (new_state.positions[player_turn] - 1 + move) % board_size + 1;
                    new_state.scores[player_turn] += new_state.positions[player_turn];

                    if (new_state.scores[player_turn] >= max_score) {
                        // don't create further new universes if the player has won - game ends
                        nwins[player_turn] += nuniverses;
                    } else {
                        // create new universes
                        new_universes[new_state] += nuniverses;
                        still_playing += nuniverses;
                    }
                }
            }
            player_turn = (player_turn + 1) % nplayers;
            Universes = new_universes;
            nuniverses_playing = still_playing;
        }

        bool done() {
            return nuniverses_playing == 0;
        }

        std::string to_string() {
            std::stringstream result;
            for (auto& [universe, count] : Universes) {
                result << count << " universes with state: " << std::endl;
                for (int i=0; i<nplayers; i++)
                    result << "    Player " << i+1 << " on space " << universe.positions[i] << " with score " << universe.scores[i] << std::endl;
            }
            return result.str();
        }

        std::vector<long int> get_wins() {
            std::vector<long int> result;
            for (int i=0; i<nplayers; i++)
                result.push_back(nwins[i]);
            return result;
        }

    private:
        std::map<UniverseState, long int> Universes;
        long int nuniverses_playing = 0;
        long int nwins[nplayers];
        int player_turn = 0;
        std::vector<long int> die_roll_splits;
};

// all possible outcomes and counts
std::vector<long int>die_rolls(const int nsides, const int nrolls) {
    const int maxscore = nsides * nrolls + 1;

    if (nrolls == 1) {
        std::vector<long int> result(maxscore, 1);
        result[0] = 0;
        return result;
    }

    std::vector<long int> result(maxscore, 0);
    std::vector<long int> prev_result = die_rolls(nsides, nrolls - 1);

    for (int side=1; side <= nsides; side++) {
        for (int score=0; score < prev_result.size(); score++) {
            result[score + side] += prev_result[score];
        }
    }

    return result;
}

std::vector<short int> starting_positions(std::ifstream& input) {
    std::vector<short int> player;
    std::vector<short int> start;
    int maxplayers = -1;

    std::string line;
    const std::regex target_re("Player (\\d+) starting position: (\\d+)");
    std::smatch smatches;

    while (std::getline(input, line)) {
        if (!std::regex_match(line, smatches, target_re))
            continue;

        int player_num = std::stoi(smatches[1])-1;
        player.push_back(player_num);
        if (player_num > maxplayers)
            maxplayers = player_num;

        start.push_back(std::stoi(smatches[2]));
    }

    std::vector<short int> result = std::vector<short int>(maxplayers+1, 0);
    for (int player_num=0; player_num <= maxplayers; player_num++) {
        for (int i=0; i<player.size(); i++)
            if (player[i] == player_num)
                result[player_num] = start[i];
    }

    return result;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    auto starts = starting_positions(input);
    auto roll_possibilities = die_rolls(3, 3);

    auto mvs = MultiverseSimulator(roll_possibilities, starts);
    while (!mvs.done()) 
        mvs.turn();

    std::cout << mvs.to_string() << std::endl;
    auto wins = mvs.get_wins();
    for (int i=0; i<wins.size(); i++)
        std::cout << wins[i] << " ";
    std::cout << std::endl;

    std::cout << "Part 2:" << std::endl;
    std::cout << "      " << *std::max_element(wins.begin(), wins.end()) << std::endl;
}
