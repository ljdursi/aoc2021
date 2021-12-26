#include <vector>
#include <string>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <regex>

bool overlap(std::pair<int, int> range1, std::pair<int, int> range2) {
    return range1.first <= range2.second && range2.first <= range1.second;
}

class ASTNode {
    public:
        enum opcodes { INPUT, ADD, MUL, DIV, MOD, EQUAL, ASSIGN };
        ASTNode(opcodes type, std::vector<ASTNode> children) : op(type), children(children), is_literal(false), is_input(false), value(0), inputs() {
            for (const auto& child: children) {
                for (const auto input: child.inputs) {
                    inputs.insert(input);
                }
            }
            std::vector<int> poss_values = {children[0].min_value * children[1].min_value, children[0].min_value * children[1].max_value,
                                            children[0].max_value * children[1].min_value, children[0].max_value * children[1].max_value};
            switch(op) {
                case ADD:
                    min_value = children[0].min_value + children[1].min_value;
                    max_value = children[0].max_value + children[1].max_value;
                    break;
                case MUL:
                    min_value = *std::min_element(poss_values.begin(), poss_values.end());
                    max_value = *std::max_element(poss_values.begin(), poss_values.end());
                    break;
                case DIV:
                    min_value = children[0].min_value / children[1].max_value;
                    max_value = (children[1].min_value > 0 ? children[0].max_value / children[1].min_value : INT_MAX);
                    break;
                case MOD:
                    min_value = 0;
                    max_value = children[1].max_value;
                    break;
                case EQUAL:
                    min_value = 0;
                    max_value = 1;
                    break;
                case ASSIGN:
                    min_value = value;
                    max_value = value;
                    break;
                default:
                    break;
            }
        };
        ASTNode(int value) : op(ASSIGN), children(), is_literal(true), is_input(false), value(value), inputs(), min_value(value), max_value(value) {}
        ASTNode(short int input) : op(INPUT), children(), is_literal(false), is_input(true), value(input), inputs({input}), min_value(1), max_value(9) {}
        ASTNode& operator=(const ASTNode& other);
        ASTNode& operator[](const short int idx) { return children[idx]; }
        bool set_value(int value) {
            if (is_literal) {
                this->value = value;
                min_value = value;
                max_value = value;
                return true;
            } else {
                return false;
            }
        }

        bool literal() const { return is_literal; };
        bool input() const { return is_input; };
        ASTNode::opcodes type() const { return op; };

        std::pair<int, int> range() const { return {min_value, max_value}; }
        std::set<short int> inputs_used() const { return inputs; };

        int eval(const std::vector<int> inputs) const;
        std::string as_code() const;

    private:
        opcodes op;
        std::vector<ASTNode> children;
        bool is_literal;
        bool is_input;
        int value;
        std::set<short int> inputs;
        int min_value;
        int max_value;
};

ASTNode& ASTNode::operator=(const ASTNode& other) {
    children.clear();
    op = other.op;
    is_literal = other.is_literal;
    is_input = other.is_input;
    value = other.value;
    min_value = other.min_value;
    max_value = other.max_value;
    inputs = other.inputs;

    std::copy(other.children.begin(), other.children.end(), std::back_inserter(children));
    return *this;
}

int ASTNode::eval(std::vector<int> inputs) const {
    switch (op) {
        case ASSIGN:
            return value;
        case INPUT:
            return inputs[value];
        case ADD:
            return children[0].eval(inputs) + children[1].eval(inputs);
        case MUL:
            return children[0].eval(inputs) * children[1].eval(inputs);
        case DIV:
            return children[0].eval(inputs) / children[1].eval(inputs);
        case MOD:
            return children[0].eval(inputs) % children[1].eval(inputs);
        case EQUAL:
            return (children[0].eval(inputs) == children[1].eval(inputs)) ? 1 : 0;
    }
    return 0;
}

std::string ASTNode::as_code() const {
    std::string s = "";
    switch (op) {
        case ASSIGN:
            s += std::to_string(value);
            break;
        case INPUT:
            s += "inputs[" + std::to_string(value) + "]";
            break;
        case ADD:
            s += "(" + children[0].as_code() + " + " + children[1].as_code() + ")";
            break;
        case MUL:
            s += "(" + children[0].as_code() + " * " + children[1].as_code() + ")";
            break;
        case DIV:
            s += "(" + children[0].as_code() + " / " + children[1].as_code() + ")";
            break;
        case MOD:
            s += "(" + children[0].as_code() + " % " + children[1].as_code() + ")";
            break;
        case EQUAL:
            s += "(" + children[0].as_code() + " == " + children[1].as_code() + " ? 1 : 0)";
            break;
    }
    return s;
}

class ALU {
    public:
        ALU() {
            for (int node = 0; node < 4; node++) {
                variables.push_back(ASTNode((int)0));
            }
        }
        void add_instruction(const std::string& instruction, const std::string& variable, const bool is_value, const std::string& variable2, const int value);
        std::vector<int> eval(const std::vector<int>& inputs) const {
            std::vector<int> result;
            for (auto var : variables) {
                result.push_back(var.eval(inputs));
            }
            return result;
        };
        std::string to_string(const std::vector<int> inputs) const {
            return "ALU: w = " + std::to_string(variables[0].eval(inputs)) + " x = " + std::to_string(variables[1].eval(inputs)) 
                     + " y = " + std::to_string(variables[2].eval(inputs)) + " z = " + std::to_string(variables[3].eval(inputs));
        }
        std::string to_code() const {
            std::string s = "";
            for (short int i=0; i<variables.size(); i++) {
                s = s + (char)('w' + (char)i) + " = " + variables[i].as_code() + ";" + "  // " + std::to_string(variables[i].range().first) + "... " + std::to_string(variables[i].range().second) + "\n";
            }
            return s;
        }
    private:
        const std::map<std::string, short int> variable = {{"w", 0}, {"x", 1}, {"y", 2}, {"z", 3}};
        std::vector<ASTNode> variables;
};

void ALU::add_instruction(const std::string& instruction, const std::string& var, const bool is_value, const std::string& variable2, const int value) {
    short int idx = variable.at(var);
    const std::map<std::string, ASTNode::opcodes> op = {{"add", ASTNode::ADD}, {"mul", ASTNode::MUL}, {"div", ASTNode::DIV}, {"mod", ASTNode::MOD}, {"eql", ASTNode::EQUAL}, 
                                                        {"inp", ASTNode::INPUT}};
    auto opcode = op.at(instruction);
    short int idx2 = (is_value || (opcode == ASTNode::INPUT)) ? -1 : variable.at(variable2);

    if (opcode == ASTNode::INPUT) {
        variables[idx] = ASTNode((short int)value);
        return;
    }

    ASTNode right_node = is_value ? ASTNode((int) value): variables[idx2];
    ASTNode new_node(opcode, {variables[idx], right_node});
    if (new_node.inputs_used().empty()) {
        auto result = new_node.eval({});
        variables[idx] = ASTNode((int)result);
        return;
    }

    // if there's a special case we can use, use that, otherwise use new_node;
    switch(opcode) {
        case ASTNode::ASSIGN:
            variables[idx] = right_node;
            break;

        case ASTNode::ADD:
            // adding zero is a no-op
            if (right_node.inputs_used().empty() && right_node.eval({}) == 0) {
                break;
            }

            if (variables[idx].literal() && variables[idx].eval({}) == 0) {
                variables[idx] = right_node;
                break;
            }

            // adding -x to x just sets everything to zero
            if (!is_value && variable2 == "-" + var) {
                variables[idx] = ASTNode((int)0);
                break;
            }

            variables[idx] = new_node;
            break;

        case ASTNode::MUL:
            // multiplying by one is a no-op
            if (right_node.inputs_used().empty() && right_node.eval({}) == 1) {
                break;
            }

            if (variables[idx].inputs_used().empty() && variables[idx].eval({}) == 1) {
                variables[idx] = right_node;
                break;
            }

            // multiplying by 0 sets the value to zero
            if (is_value && value == 0) {
                variables[idx] = ASTNode((int)0);
                break;
            }

            if (variables[idx].literal() && variables[idx].eval({}) == 0) {
                variables[idx] = ASTNode((int)0);
                break;
            }

            variables[idx] = new_node;
            break;

        case ASTNode::DIV:
            // dividing by one is a no-op
            if (right_node.inputs_used().empty() && right_node.eval({}) == 1) {
                break;
            }

            // dividing by self sets value to 1
            if (!is_value && (variable2 == var)) {
                variables[idx] = ASTNode((int)1);
                break;
            }

            // dividing by a number 
            if (!is_value && variables[idx].range().second < value && abs(variables[idx].range().first) < value) {
                variables[idx] = ASTNode((int)0);
                break;
            }

            // if we're dividing by something we've recently multiplied by the same number,
            // we can cancel out the operations
            if (right_node.inputs_used().empty()) {
                int div_val = right_node.eval({});

                if (variables[idx].type() == ASTNode::ADD) {
                    if (variables[idx][0].type() == ASTNode::MUL) {
                        if (variables[idx][0][1].inputs_used().empty() && variables[idx][0][1].eval({}) == div_val) {
                            auto right_range = variables[idx][1].range();
                            if ((abs(right_range.first) < div_val) && (abs(right_range.second) < div_val)) {
                                auto copy = variables[idx][0][0];
                                variables[idx] = copy;
                                break;
                            }
                        }
                    }
                }
            }

            variables[idx] = new_node;
            break;

        case ASTNode::EQUAL:
            // testing for equality against self sets value to 1
            if (!is_value && (variable2 == var)) {
                variables[idx] = ASTNode((int)1);
                break;
            }

            // we know that inputs are between 1 and 9 inclusive
            if (is_value && (value < 1 || value > 9) && variables[idx].input()) {   
                variables[idx] = ASTNode((int)0);
                break;
            }

            if (right_node.input() && variables[idx].literal() && (variables[idx].eval({}) < 1 || variables[idx].eval({}) > 9)) {
                variables[idx] = ASTNode((int)0);
                break;
            }

            // if there's no overlap between the two ranges, must be zero
            if (!overlap(variables[idx].range(), right_node.range())) {
                variables[idx] = ASTNode((int)0);
                break;
            }

            variables[idx] = new_node;
            break;

        case ASTNode::MOD:
            if (right_node.inputs_used().empty()) {
                int mod_val = right_node.eval({});
                const auto& [min_val, max_val] = variables[idx].range();

                if (abs(min_val) < mod_val && abs(max_val) < mod_val) {
                    break;
                }
            }

            // if we're modding by something we've recently multiplied by the same number,
            // we can just set the value to zero
            if (right_node.inputs_used().empty()) {
                int mod_val = right_node.eval({});

                if (variables[idx].type() == ASTNode::ADD) {
                    if (variables[idx][0].type() == ASTNode::MUL) {
                        if (variables[idx][0][1].inputs_used().empty() && variables[idx][0][1].eval({}) == mod_val) {
                            variables[idx] = ASTNode(ASTNode::ADD, {ASTNode((int)0), variables[idx][1]});
                            variables[idx] = ASTNode(opcode, {variables[idx], right_node});
                            break;
                        }
                    }
                }
            }

            variables[idx] = new_node;
            break;

        case ASTNode::INPUT:
            variables[idx] = ASTNode((short int)value);
            break;
    }
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

    ALU alu;
    std::string line;
    short int input_idx = 0;
    while(std::getline(input, line)) {
        const std::regex input_regex("inp ([wxzy])");
        std::smatch smatch;
        if (std::regex_match(line, smatch, input_regex)) {
            alu.add_instruction("inp", smatch[1], false, "", input_idx);
            input_idx++;
            continue;
        }

        const std::regex op_literal_regex("(add|mul|div|mod|eql) ([wxzy]) (\\d+)");
        if (std::regex_match(line, smatch, op_literal_regex)) {
            alu.add_instruction(smatch[1], smatch[2], true, "", std::stoi(smatch[3]));
            continue;
        }

        const std::regex op_variable_regex("(add|mul|div|mod|eql) ([wxzy]) ([wxyz])");
        if (std::regex_match(line, smatch, op_variable_regex)) {
            alu.add_instruction(smatch[1], smatch[2], false, smatch[3], 0);
            continue;
        }
    }

    std::cout << alu.to_code() << std::endl;
}