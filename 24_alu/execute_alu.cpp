#include <string>
#include <iostream>

std::string to_string(const int inputs[14]) {
    std::string s = "";
    for (int i=0; i<14; i++) {
        s += std::to_string(inputs[i]);
    }
    return s;
}

int main() {
    int inputs[14] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    long int count = 0;

    int w, x, y, z;
    while (true) {
        if (count++ % 100000000 == 0) {
            std::cout << "Trying " << to_string(inputs) << std::endl;
            count = 1l;
        }

        // add line output from alu.cpp here
    	#include "output.c"

        if (z == 0l) {
            std::cout << "Maximum valid value = " << to_string(inputs) << std::endl;
            break;
        }

        short int digit=13;
        while (true) {
            inputs[digit]--;
            if (inputs[digit] > 0)
                break;

            inputs[digit] = 9;
            digit--;
        }
    }
}
