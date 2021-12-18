#include <initializer_list>
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

class SFNumber {
    public:
        SFNumber(const int n) : is_literal(true), literal(n) {}
        SFNumber(const int left, const int right) : is_literal(false), left(new SFNumber(left)), right(new SFNumber(right)) {}
        SFNumber(const std::vector<std::string>& tokens);
        SFNumber(const std::string s);
        SFNumber(SFNumber* a, SFNumber *b);
        SFNumber add(SFNumber* b);
        void reduce();
        std::string to_string() const;
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
        bool explode();
        ~SFNumber();
        SFNumber(std::vector<std::smatch>& smatches);
        static const int MAX_LEVEL = 4;
        static const int MAX_LITERAL = 9;

    private:
        SFNumber *predecessor();
        SFNumber *successor();
        void increment_depth() { level++; for (auto child: {left, right}) child->increment_depth(); }
        SFNumber *find_exploder();
        SFNumber *find_splitter();

        bool is_literal;
        short int literal;
        short int level;
        SFNumber *left = nullptr;
        SFNumber *right = nullptr;
        SFNumber *parent = nullptr;
        bool is_left_child = false;
        bool is_right_child = false;
};

SFNumber::~SFNumber() {
    if (left != nullptr)
        delete left;
    if (right != nullptr)
        delete right;    
};

SFNumber *SFNumber::predecessor() {
    if (parent == nullptr )
        return nullptr;

    // walk up the tree until we can go left
    // then walk back down the tree to the rightmost non-literal child of that left subtree
    SFNumber *p = this;
    while (p != nullptr && p->is_left_child) {
        p = p->parent;
    }
    if (p == nullptr)
        return nullptr;

    p = p->parent->left;
    while ((p != nullptr) && !(p->right->is_literal)) {
        p = p->right;
    }
    return p;
}

SFNumber *SFNumber::successor() {
    if (parent == nullptr )
        return nullptr;

    // walk up the tree until we can go right
    // then walk back down the tree to the leftmost non-literal child of that right subtree
    SFNumber *p = this;
    while (p != nullptr && p->is_right_child) {
        p = p->parent;
    }
    if (p == nullptr)
        return nullptr;

    p = p->parent->right;
    while (p->left->is_literal) {
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
        lneigh->right->literal += left->literal;
    }
    delete left;

    auto rneigh = successor();
    if (rneigh != nullptr) {
        rneigh->left->literal += right->literal;
    }
    delete right;

    is_literal = true;
    literal = 0;

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
        }
        SFNumber* splitter = find_splitter();
        if (splitter != nullptr) {
            splitter->split();
            changed = true;
        }
    } while (changed);
}

SFNumber::SFNumber(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        is_literal = true;
        literal = std::stoi(tokens[0]);
    } else {
        is_literal = false;
        left = new SFNumber(tokens[0]);
        right = new SFNumber(tokens[1]);
        for (SFNumber *child : {left, right}) {
            child->level = level+1;
            child->parent = this;
        }
    }
}

int main() {

    std::string s = "[[1,2],[3,4]]";
    std::regex tokens_re("(\[|\\d+|,|\])");
    std::smatch smatches;
    std::regex_match(s, smatches, tokens_re);
    auto n = SFNumber(smatches);

    std::cout << n.to_string() << std::endl;
}