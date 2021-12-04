#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

std::vector<int> called_numbers(std::string line) {
    std::vector<int> numbers;

    std::stringstream ss(line);
    std::string number;
    while (std::getline(ss, number, ',')) {
        numbers.push_back(std::stoi(number));
    }

    return numbers;
}

class Bingo {
    public:
        Bingo(std::vector<int> numbers) : bingo_board(numbers) {
            for (int row=0; row<5; row++) 
                for (int col=0; col<5; col++) 
                    called[row][col] = false;
        };
        bool call(const int number);
        int sum_unmarked();

        void print() {
            int ncalled = 0;
            for (int row=0; row<5; row++) {
                for (int col=0; col<5; col++) {
                    if (called[row][col]) {
                        ncalled++;
                        std::cout << " (" << std::setw(2) << bingo_board[row*5 + col] << ")";
                    } else {
                        std::cout << std::setw(4) << bingo_board[row*5 + col] << " ";
                    }
                }
                std::cout << std::endl;
            }
            std::cout << "ncalled: " << ncalled << std::endl;
        }

    private:
        std::vector<int> bingo_board;
        bool called[5][5];
        bool check_row(int row) {
            for (int i=0; i<5; i++) {
                if (!called[row][i]) {
                    return false;
                }
            }
            return true;
        }
        bool check_col(int col) {
            for (int i=0; i<5; i++) {
                if (!called[i][col]) {
                    return false;
                }
            }
            return true;
        }
};

bool Bingo::call(const int number) {
    auto found = std::find(bingo_board.begin(), bingo_board.end(), number);
    if (found != bingo_board.end()) {
        const int idx = std::distance(bingo_board.begin(), found);
        const int row = idx / 5, col = idx % 5;
        called[row][col] = true;

        if (check_row(row)) {
            return true;
        }
        if (check_col(col)) {
            return true;
        }
    }
    return false;
}

int Bingo::sum_unmarked() {
    int sum = 0;
    for (int row=0; row<5; row++) {
        for (int col=0; col<5; col++) {
            if (!called[row][col]) {
                sum += bingo_board[row*5 + col];
            }
        }
    }
    return sum;
}


int main() {
    std::string line;

    std::cin >> line;
    std::vector<int> calls = called_numbers(line);

    std::vector<int> board_numbers;
    int num;
    while (std::cin >> num) {
        board_numbers.push_back(num);
    }

    const int nboards = board_numbers.size() / 25;
    std::vector<Bingo> boards;

    for (int i=0; i<nboards; i++) {
        std::vector<int> subset = std::vector<int>(board_numbers.begin() + i*25, board_numbers.begin() + (i+1)*25);
        boards.push_back(Bingo(subset));
    }

    std::vector<int> winners;
    std::vector<bool> in_play(nboards, true); 

    for (const int call: calls) {
        for (int i=0; i<nboards; i++) {
            if (!in_play[i]) 
                continue;

            auto &board=boards[i];
            if (board.call(call)) {
                int score = board.sum_unmarked()*call;
                std::cout << "     BINGO! on card " << i << std::endl;
                std::cout << "     score = " << score << std::endl;
                winners.push_back(score);
                in_play[i] = false;
            }
        }
    }

    std::cout << "Part 1:" << std::endl;
    std::cout << "  first winner: " << winners[0] << std::endl;

    std::cout << "Part 2:" << std::endl;
    std::cout << "  last winner: " << winners[nboards-1] << std::endl;
}