#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

class Die {
    public:
        Die(int sides) : sides(sides), nrolls(0) {};
        virtual int roll() { nrolls++; return rand() % sides + 1; };
        virtual int num_rolls() const { return nrolls; }
        int num_sides() const { return sides; }
    protected:
        int sides;
        int nrolls;
};

class DeterministicDie : public Die {
    public:
        DeterministicDie(int sides, int value) : Die(sides), value(value) {}
        int roll() override {
            Die::roll();
            int result = value;
            value = (value % sides) + 1;
            return result;
        };
    private:
        int value;
};

class DiracDice { 
    public:
        DiracDice(Die* die, std::vector<int> starting_positions) : die(die) {
            for (int i=0; i<nplayers; i++) {
                player_positions[i] = starting_positions[i];
                player_scores[i] = 0;
            }
        }

        void turn() {
            int move = 0;
            for (int i=0; i<nrolls_per_turn; i++)
                move += die->roll();

            player_positions[player_turn] = (player_positions[player_turn] - 1 + move) % board_size + 1;
            player_scores[player_turn] += player_positions[player_turn];

            player_turn = (player_turn + 1) % nplayers;
        }

        bool won() {
            for (int i=0; i<nplayers; i++)
                if (player_scores[i] >= max_score)
                    return true;
            return false;
        }

        long int game_score() {
            int loser = 0;
            for (int i=1; i<nplayers; i++)
                if (player_scores[i] < player_scores[loser])
                    loser = i;

            return player_scores[loser]*(die->num_rolls());
        }

        std::string to_string() const {
            std::stringstream result;
            for (int i=0; i<nplayers; i++)
                result << "Player " << i+1 << " on space " << player_positions[i] << " with score " << player_scores[i] << "\n";
            return result.str();
        }

    private:
        static const int nplayers = 2;
        static const int board_size = 10;
        static const int max_score = 1000;
        static const int nrolls_per_turn = 3;
        int player_positions[nplayers];
        int player_scores[nplayers];
        int player_turn = 0;
        Die *die;
};

std::vector<int> starting_positions(std::ifstream& input) {
    std::vector<int> player;
    std::vector<int> start;
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

    std::vector<int> result = std::vector<int>(maxplayers+1, 0);
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

    DeterministicDie die(100, 1);
    DiracDice game(&die, starts);

    while (!game.won()) {
        game.turn();
    }

    std::cout << "Part 1:" << std::endl;
    std::cout << "      Game score: " << game.game_score() << std::endl;
}
