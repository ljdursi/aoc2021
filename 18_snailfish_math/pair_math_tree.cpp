#include <initializer_list>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <regex>

class SFNumber {
    public:
        SFNumber(const std::vector<std::string>& tokens, SFNumber *parent=nullptr, const bool left=false);
        SFNumber(const int n, SFNumber *parent=nullptr) : is_literal(true), literal(n), parent(parent) { if (parent != nullptr) level = parent->level += 1; else level = 0;};
        SFNumber(SFNumber& a, SFNumber &b, SFNumber *parent=nullptr);

        SFNumber(const SFNumber& other);
        SFNumber(SFNumber&& other);
        SFNumber& operator=(const SFNumber& other);

        void add(const SFNumber& b);

        void reduce();
        std::string to_string() const;
        std::string print_as_tree(const std::string& prefix="") const;
        long int magnitude() const {
            if (is_literal)
                return literal;
            else
                return 3l*left->magnitude() + 2l*right->magnitude();
        }
        bool split() {
            if (is_literal && literal > MAX_LITERAL) {
                is_literal = false;
                left = new SFNumber(literal/2); left->is_left_child=true;
                right = new SFNumber((literal+1)/2); right->is_right_child=true;
                for (SFNumber *child : {left, right}) {
                    child->level = level+1;
                    child->parent = this;
                }
                return true;
            } else {
                return false;
            }
        }
        static std::vector<std::string> tokenize(const std::string& s);
        bool explode();
        ~SFNumber();
        static const int MAX_LEVEL = 4;
        static const int MAX_LITERAL = 9;

    private:
        SFNumber *predecessor();
        SFNumber *successor();
        void increment_depth() { level++; for (auto child: {left, right}) if (child) child->increment_depth(); }
        SFNumber *find_exploder();
        SFNumber *find_splitter();
        static int find_divider(const std::vector<std::string>& tokens);

        bool is_left_child = false;
        bool is_right_child = false;
        bool is_literal;
        short int literal;
        short int level;
        SFNumber *left = nullptr;
        SFNumber *right = nullptr;
        SFNumber *parent = nullptr;
};

SFNumber::~SFNumber() {
    delete left;
    delete right;
};

SFNumber *SFNumber::predecessor() {
    if (parent == nullptr)
        return nullptr;

    // walk up the tree until we can go left
    SFNumber *p = this;
    while (p != nullptr && p->is_left_child) {
        p = p->parent;
    }
    if (p == nullptr)
        return nullptr;

    if (p->parent == nullptr)
        return nullptr;
    
    // then walk back down the tree to the rightmost (literal) child of that left subtree
    p = p->parent->left;
    while ((p != nullptr) && (p->right != nullptr)) {
        p = p->right;
    }
    return p;
}

SFNumber *SFNumber::successor() {
    if (parent == nullptr)
        return nullptr;

    // walk up the tree until we can go right
    SFNumber *p = this;
    while (p != nullptr && p->is_right_child) {
        p = p->parent;
    }
    if (p == nullptr)
        return nullptr;

    if (p->parent == nullptr)
        return nullptr;

    // then walk back down the tree to the leftmost (literal) child of that right subtree
    p = p->parent->right;
    while ((p != nullptr) && (p->left != nullptr)) {
        p = p->left;
    }
    return p;
}

std::string SFNumber::to_string() const {
    std::stringstream ss;
    if (is_literal) {
        ss << literal;
    } else {
        ss << "[" << left->to_string() << "," << right->to_string() << "]";
    }
    return ss.str();
}

bool SFNumber::explode() {
    if (is_literal) {
        std::cout << "Cannot explode literal " << literal << std::endl;
        return false;
    }

    if (level <= MAX_LEVEL) {
        std::cout << "Cannot explode " << to_string() << " at level " << level << std::endl;
        return false;
    }

    if (!(left->is_literal) || !(right->is_literal)) {
        std::cout << "Cannot explode " << to_string() << " mid-tree " << std::endl;
        return false;
    }

    auto lneigh = predecessor();
    if (lneigh != nullptr) {
        lneigh->literal += left->literal;
    }

    auto rneigh = successor();
    if (rneigh != nullptr) {
        rneigh->literal += right->literal;
    }

    delete left;
    delete right;

    is_literal = true;
    literal = 0;
    left = nullptr;
    right = nullptr;

    return true;
}

SFNumber* SFNumber::find_exploder() {
    // returns the first node that needs exploding - nested too deep
    if (is_literal)
        return nullptr;

    if (left->is_literal && right->is_literal && level > MAX_LEVEL)
        return this;

    auto exploder = left->find_exploder();
    if (exploder != nullptr)
        return exploder;
    else
        return right->find_exploder();
}

SFNumber* SFNumber::find_splitter() {
    // returns the first node that needs exploding - literal too large
    if (is_literal) {
        if (literal > MAX_LITERAL)
            return this;
        else
            return nullptr;
    } 

    auto splitter = left->find_splitter();
    if (splitter != nullptr)
        return splitter;
    else
        return right->find_splitter();
}

void SFNumber::reduce() {
    if (is_literal)
        return;

    bool changed = false;
    do {
        changed = false;
        SFNumber* exploder = find_exploder();
        if (exploder != nullptr) {
            exploder->explode();
            changed = true;
            continue;
        }
        SFNumber* splitter = find_splitter();
        if (splitter != nullptr) {
            splitter->split();
            changed = true;
        }
    } while (changed);
}

int SFNumber::find_divider(const std::vector<std::string>& tokens) {
    int level = 0, divider=-1;
    for (int i=0; i<tokens.size(); i++) {
        if (tokens[i] == "[") {
            level++;
        } else if (tokens[i] == "]") {
            level--;
        } else if (tokens[i] == "," && level == 1) {
            divider = i;
            break;
        }
    }
    return divider;
}

SFNumber::SFNumber(const std::vector<std::string>& tokens, SFNumber *parent, const bool left_child) : parent(parent) {
    if (parent != nullptr) {
        level = parent->level + 1;
        is_left_child = left_child;
        is_right_child = !left_child;
    } else {
        level = 1;
        is_left_child = false;
        is_right_child = false;
    }

    if (tokens.size() == 1) {
        is_literal = true;
        literal = std::stoi(tokens[0]);
    } else {
        is_literal = false;
        int divider = find_divider(tokens);
        
        std::vector<std::string> left_tokens(tokens.begin()+1, tokens.begin()+divider);
        std::vector<std::string> right_tokens(tokens.begin()+(divider+1), tokens.end()-1);
        left = new SFNumber(left_tokens, this, true);
        right = new SFNumber(right_tokens, this, false);
    }
}

std::vector<std::string> SFNumber::tokenize(const std::string& s) {
    std::vector<std::string> tokens;
    std::regex tokens_re("(\\[|\\d+|,|\\])");

    auto tokens_begin = std::sregex_iterator(s.begin(), s.end(), tokens_re);
    auto tokens_end = std::sregex_iterator();

    for (auto it = tokens_begin; it != tokens_end; ++it) {
        std::smatch match = *it;
        std::string token = match.str();
        tokens.push_back(token);
    }    

    return tokens;
}

SFNumber::SFNumber(SFNumber&& other) : is_left_child(other.is_left_child), is_right_child(other.is_right_child),
                                       is_literal(other.is_literal), literal(other.literal), level(other.level),
                                       left(other.left), right(other.right), parent(other.parent) {
    other.left = nullptr;
    other.right = nullptr;
    other.is_literal = true;
    other.parent = nullptr;
}

SFNumber::SFNumber(const SFNumber& other) : is_left_child(other.is_left_child), is_right_child(other.is_right_child),
                                            is_literal(other.is_literal), literal(other.literal), level(other.level),
                                            parent(other.parent) {
    if (!is_literal) {
        left = new SFNumber(*other.left);
        left -> parent = this;
        right = new SFNumber(*other.right);
        right -> parent = this;
    } else {
        left = nullptr;
        right = nullptr;
    }
}

SFNumber& SFNumber::operator=(const SFNumber& other) {
    is_left_child = other.is_left_child;
    is_right_child = other.is_left_child;
    parent = other.parent;
    level = other.level;

    SFNumber *orig_left = left;
    SFNumber *orig_right = right;

    left = new SFNumber(*other.left);
    right = new SFNumber(*other.right);

    delete orig_left;
    delete orig_right;
    return *this;
}

void SFNumber::add(const SFNumber& b) {
    SFNumber *new_left = new SFNumber(*this);

    SFNumber *orig_left = left;
    SFNumber *orig_right = right;

    left = new_left;
    left->increment_depth();
    left->parent = this;
    left->is_left_child = true;

    right = new SFNumber(b);
    right->increment_depth();
    right->parent = this;
    right->is_right_child = true;

    is_literal = false;
    literal = 0;

    delete orig_left;
    delete orig_right;
}

std::string SFNumber::print_as_tree(const std::string& prefix) const {
    std::stringstream ss;
    std::string child_type = is_left_child ? "left_child" : (is_right_child ? "right_child" : "unknown_child");
    bool parent_confirmed = false;
    if (parent != nullptr) {
        if (is_left_child && parent->left == this)
            parent_confirmed = true;
        if (is_right_child && parent->right == this)
            parent_confirmed = true;
    }
    std::string parent_confirmed_str = "no parent";
    if (parent != nullptr)
        parent_confirmed_str = parent_confirmed ? "confirmed" : "WRONG!!";
    if (is_literal) {
        ss << prefix << std::to_string(literal) << "  " << child_type << "  parent: " << parent << "  " << parent_confirmed_str << std::endl;
    } else {
        ss << prefix << this << " " << child_type << "  parent: " << parent << "  " << parent_confirmed_str << std::endl;
        ss << prefix << "Left child: " << left << std::endl;
        ss << left->print_as_tree(prefix+"  ");
        ss << prefix << "Right child: " << right << std::endl;
        ss << right->print_as_tree(prefix+"  ");
    }
    return ss.str();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    std::string line;
    std::vector<SFNumber> numbers;
    while (std::getline(input, line)) {
        auto number = SFNumber(SFNumber::tokenize(line));
        numbers.push_back(number);
    }

    SFNumber result = numbers[0];
    for (int i=1; i<numbers.size(); i++) {
        result.add(numbers[i]);
        result.reduce();
    }

    std::cout << "Part 1: " << std::endl;
    std::cout << "    after addition: " << result.to_string() << std::endl;
    std::cout << "    magnitude     : " << result.magnitude() << std::endl;

    // std::cout << "Part 2: " << std::endl;
    // long int max_mag = LONG_MIN;
    // for (auto& n1: numbers) {
    //     for (auto& n2: numbers) {
    //         SFNumber n3 = n1;
    //         n3.add(n2);
    //         n3.reduce();
    //         if (n3.magnitude() > max_mag) {
    //             max_mag = n3.magnitude();
    //         }
    //     }
    // }
    // std::cout << "    max magnitude : " << max_mag << std::endl;
}