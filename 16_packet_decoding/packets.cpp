#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <numeric>

const int bits_per_char = 4;  // hex representation
const std::map<char, short int> hex_lookup = 
    {{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7},
     {'8', 8}, {'9', 9}, {'A', 10}, {'B', 11}, {'C', 12}, {'D', 13}, {'E', 14}, {'F', 15}};

char byte_to_char(short int byte, const short int nbits) {
    if (nbits < bits_per_char) {
        byte <<= (bits_per_char - nbits);
    }
    return (byte < 10) ? '0' + byte : 'A' + byte - 10;
}

typedef struct bitstream_cursor {
    int char_idx;
    int bit_idx;
    bitstream_cursor(int char_idx=0, int bit_idx=bits_per_char-1) : char_idx(char_idx), bit_idx(bit_idx) {}
} bitstream_cursor_t;

class Bitstream {
    public:
        Bitstream(const std::string& hex_input, const int nbits);
        unsigned long int next_n_bits_as_int(const int n_bits);
        Bitstream next_n_bits_as_bitstream(const int n_bits);
        bool bits_left() const;
        std::string to_string() const;
    private:
        const std::string hex_input;
        bitstream_cursor_t cursor;
        std::vector<bool> next_n_bits(const int n_bits);
        short int byte_at(const int idx) const;
        int _nbits;
};

Bitstream::Bitstream(const std::string& hex_input, const int nbits=-1) : hex_input(hex_input), cursor() {
    if (nbits == -1) {
        _nbits = hex_input.length() * bits_per_char;
    } else {
        _nbits = nbits;
    }
}

bool Bitstream::bits_left() const {
    auto bit_posn = cursor.char_idx * bits_per_char + ((bits_per_char-1) - cursor.bit_idx);
    return bit_posn < _nbits;
}

std::string Bitstream::to_string() const {
    std::stringstream ss;
    auto bit_posn = cursor.char_idx * bits_per_char + ((bits_per_char-1) - cursor.bit_idx);
    ss << "\"" << hex_input << "\"" << "  " << _nbits << " bits, cur_position = " << cursor.char_idx << ":" << cursor.bit_idx << " (" << bit_posn << ")" << std::endl;
    return ss.str();
}

short int Bitstream::byte_at(const int idx) const {
    if (idx < 0 || idx >= hex_input.length()) {
        return 0;
    }
    return hex_lookup.at(hex_input[idx]);
}

unsigned long int Bitstream::next_n_bits_as_int(const int n_bits) {
    unsigned long int result = 0ul;
    int nbits_remaining = n_bits;
    short int cur_char = byte_at(cursor.char_idx);

    while (nbits_remaining > 0) {
        result <<= 1;
        result += (cur_char & 1<<(cursor.bit_idx)) ? 1 : 0;

        nbits_remaining--;
        cursor.bit_idx--;
        if (cursor.bit_idx < 0) {
            cursor.bit_idx = bits_per_char-1;
            cursor.char_idx++;
            cur_char = byte_at(cursor.char_idx);
        }
    }

    return result;
}

Bitstream Bitstream::next_n_bits_as_bitstream(const int n_bits) {
    std::string new_input;
    int nbits_remaining = n_bits;

    while (nbits_remaining > 0) {
        auto nbits_taken = (nbits_remaining >= 4) ? 4 : nbits_remaining;
        short int byte = (short int)(next_n_bits_as_int(nbits_taken) & 0xF);

        new_input += byte_to_char(byte, nbits_taken);
        nbits_remaining -= nbits_taken;
    }

    return Bitstream(new_input, n_bits);
}


class Packet {
    public:
        Packet(Bitstream& bitstream);
        std::string to_string(const std::string& prefix) const;
        int version() const { return _version; };
        int type() const { return _type; };
        long int value() const;
        int version_sum() const;
        static const int SUM = 0;
        static const int PRODUCT = 1;
        static const int MINIMUM = 2;
        static const int MAXIMUM = 3;
        static const int LITERAL = 4;
        static const int GREATER_THAN = 5;
        static const int LESS_THAN = 6;
        static const int EQUAL = 7;

    private:
        unsigned int _version;
        unsigned int _type;
        unsigned long int _literal_value;
        std::vector<Packet> subpackets;

        unsigned long int decode_literal(Bitstream& bitstream);
        void decode(Bitstream& bitstream);
};

long int Packet::value() const {
    if (_type == LITERAL)
        return _literal_value;

    std::vector<long int> sub_values;
    std::transform(subpackets.begin(), subpackets.end(),
                   std::back_inserter(sub_values),
                   [](const Packet& p) { return p.value(); });

    switch (_type) {
        case SUM:
            return std::accumulate(sub_values.begin(), sub_values.end(), 0l);
        case PRODUCT:
            return std::accumulate(sub_values.begin(), sub_values.end(), 1l, std::multiplies<long int>());
        case MINIMUM:
            return *std::min_element(sub_values.begin(), sub_values.end());
        case MAXIMUM:
            return *std::max_element(sub_values.begin(), sub_values.end());
        case GREATER_THAN:
            return (sub_values[0] > sub_values[1]) ? 1l : 0l;
        case LESS_THAN:
            return (sub_values[0] < sub_values[1]) ? 1l : 0l;
        case EQUAL:
            return (sub_values[0] == sub_values[1]) ? 1l : 0l;
    }

    return 0l;
}

int Packet::version_sum() const {
    int sum = 0;
    sum += _version;
    for (const auto& subpacket : subpackets) {
        sum += subpacket.version_sum();
    }
    return sum;
}

std::string Packet::to_string(const std::string& prefix="") const {
    std::stringstream ss;
    ss << prefix << _version << " " << _type;
    if (_type == LITERAL) {
        ss << " " << _literal_value << std::endl;
    } else {
        ss << std::endl; 
        for (const auto& subpacket : subpackets) {
            ss << subpacket.to_string(prefix + "  ");
        }
    }
    return ss.str();
}

unsigned long int Packet::decode_literal(Bitstream& bitstream) {
    bool keep_reading = true;
    unsigned long int result = 0;

    while (keep_reading) {
        unsigned long int data = bitstream.next_n_bits_as_int(bits_per_char+1);
        result <<= bits_per_char;
        result += data & (0xF);
        if (!(data & 0x10)) 
            keep_reading = false;
    }
    return result;
}

Packet::Packet(Bitstream& bitstream) {
    _version = bitstream.next_n_bits_as_int(3);
    _type = bitstream.next_n_bits_as_int(3);

    if (_type == LITERAL) {
        _literal_value = decode_literal(bitstream);
        return;
    }

    int length_type = bitstream.next_n_bits_as_int(1);
    if (length_type == 0) {
        const auto nbits = bitstream.next_n_bits_as_int(15);
        Bitstream substream = bitstream.next_n_bits_as_bitstream(nbits);
        while (substream.bits_left()) {
            subpackets.push_back(Packet(substream));
        }
        return;
    }

    if (length_type == 1) {
        const auto npackets = bitstream.next_n_bits_as_int(11);
        for (int subpacket=0; subpacket < npackets; subpacket++) {
            subpackets.push_back(Packet(bitstream));
        }
        return;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }
    std::ifstream input_file(argv[1]);
    std::string input_string;

    std::getline(input_file, input_string);

    Bitstream bitstream(input_string);
    Packet packet(bitstream);

    std::cout << "Part 1:" << std::endl;
    std::cout << "     Version sum: " << packet.version_sum() << std::endl;

    std::cout << "Part 2:" << std::endl;
    std::cout << "     Value: " << packet.value() << std::endl;
}