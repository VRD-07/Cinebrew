#include "vm.h"
#include <sstream>
#include <stdexcept>

VM::VM() {
    pc = 0;
}

void VM::push(int value) {
    stack.push_back(value);
}

int VM::pop() {
    if (stack.empty()) {
        std::cerr << "ERROR: Attempted to pop from empty stack at PC=" << pc << std::endl;
        throw std::runtime_error("Stack underflow");
    }
    int value = stack.back();
    stack.pop_back();
    return value;
}

std::vector<std::string> VM::split(const std::string& s) {
    std::vector<std::string> parts;
    std::string cur;
    bool in_quote = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '"') {
            in_quote = !in_quote;
            cur.push_back(c);
        } else if (std::isspace(static_cast<unsigned char>(c)) && !in_quote) {
            if (!cur.empty()) {
                parts.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) parts.push_back(cur);
    return parts;
}

void VM::preprocess(std::vector<std::string>& program) {
    labels.clear();

    for (int i = 0; i < (int)program.size(); i++) {
        std::string line = program[i];
        if (!line.empty() && line.back() == ':') {
            std::string label_name = line.substr(0, line.size() - 1);
            labels[label_name] = i;
        }
    }
}

void VM::execute(const std::string& instruction, std::vector<std::string>& program) {
    if (instruction.empty() || instruction.back() == ':') {
        pc++;
        return;
    }

    auto parts = split(instruction);
    if (parts.empty()) {
        pc++;
        return;
    }

    std::string opcode = parts[0];

    if (opcode == "PUSH") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: PUSH requires a value at PC=" << pc << std::endl;
            pc++;
            return;
        }
        try {
            int value = std::stoi(parts[1]);
            push(value);
            pc++;
        } catch (const std::invalid_argument&) {
            // Non-numeric literal: if immediately followed by PRINT, output directly
            std::string literal;
            size_t sep = instruction.find_first_of(" \t");
            if (sep != std::string::npos) {
                literal = instruction.substr(sep + 1);
                while (!literal.empty() && std::isspace(static_cast<unsigned char>(literal.front()))) literal.erase(literal.begin());
                while (!literal.empty() && std::isspace(static_cast<unsigned char>(literal.back()))) literal.pop_back();
            } else {
                literal = parts[1];
            }
            if (literal.size() >= 2 && literal.front() == '"' && literal.back() == '"') {
                literal = literal.substr(1, literal.size() - 2);
            }

            int next_i = pc + 1;
            while (next_i < (int)program.size() && !program[next_i].empty() && program[next_i].back() == ':') {
                next_i++;
            }

            if (next_i < (int)program.size()) {
                auto next_parts = split(program[next_i]);
                if (!next_parts.empty() && next_parts[0] == "PRINT") {
                    std::cout << literal << std::endl;
                    pc = next_i + 1;
                    return;
                }
            }

            std::cerr << "WARNING: PUSH of non-integer '" << parts[1] << "' at PC=" << pc << " - treating as 0" << std::endl;
            push(0);
            pc++;
        }
    }

    else if (opcode == "ADD") {
        int b = pop();
        int a = pop();
        push(a + b);
        pc++;
    }

    else if (opcode == "SUB") {
        int b = pop();
        int a = pop();
        push(a - b);
        pc++;
    }

    else if (opcode == "MUL") {
        int b = pop();
        int a = pop();
        push(a * b);
        pc++;
    }

    else if (opcode == "DIV") {
        int b = pop();
        int a = pop();
        if (b == 0) {
            std::cerr << "ERROR: Division by zero at PC=" << pc << std::endl;
            push(0);
        } else {
            push(a / b);
        }
        pc++;
    }

    else if (opcode == "STORE") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: STORE requires variable name at PC=" << pc << std::endl;
            pc++;
            return;
        }
        std::string var_name = parts[1];
        int value = pop();
        vars[var_name] = value;
        pc++;
    }

    else if (opcode == "LOAD") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: LOAD requires variable name at PC=" << pc << std::endl;
            pc++;
            return;
        }
        std::string var_name = parts[1];

        if (vars.find(var_name) == vars.end()) {
            std::cerr << "WARNING: Variable '" << var_name << "' not found, using 0 at PC=" << pc << std::endl;
            push(0);
        } else {
            push(vars[var_name]);
        }
        pc++;
    }

    else if (opcode == "EQ") {
        int b = pop();
        int a = pop();
        push(a == b ? 1 : 0);
        pc++;
    }

    else if (opcode == "GT") {
        int b = pop();
        int a = pop();
        push(a > b ? 1 : 0);
        pc++;
    }

    else if (opcode == "LT") {
        int b = pop();
        int a = pop();
        push(a < b ? 1 : 0);
        pc++;
    }

    else if (opcode == "NE") {
        int b = pop();
        int a = pop();
        push(a != b ? 1 : 0);
        pc++;
    }

    else if (opcode == "JMP") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: JMP requires label name at PC=" << pc << std::endl;
            pc++;
            return;
        }
        std::string label = parts[1];

        if (labels.find(label) == labels.end()) {
            std::cerr << "ERROR: Label '" << label << "' not found at PC=" << pc << std::endl;
            pc++;
            return;
        }

        pc = labels[label];
    }

    else if (opcode == "JZ") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: JZ requires label name at PC=" << pc << std::endl;
            pc++;
            return;
        }
        std::string label = parts[1];
        int value = pop();

        if (value == 0) {
            if (labels.find(label) == labels.end()) {
                std::cerr << "ERROR: Label '" << label << "' not found at PC=" << pc << std::endl;
                pc++;
            } else {
                pc = labels[label];
            }
        } else {
            pc++;
        }
    }

    else if (opcode == "JNZ") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: JNZ requires label name at PC=" << pc << std::endl;
            pc++;
            return;
        }
        std::string label = parts[1];
        int value = pop();

        if (value != 0) {
            if (labels.find(label) == labels.end()) {
                std::cerr << "ERROR: Label '" << label << "' not found at PC=" << pc << std::endl;
                pc++;
            } else {
                pc = labels[label];
            }
        } else {
            pc++;
        }
    }

    else if (opcode == "CALL") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: CALL requires label name at PC=" << pc << std::endl;
            pc++;
            return;
        }

        std::string label = parts[1];
        int argc = 0;
        if (parts.size() >= 3) {
            argc = std::stoi(parts[2]);
        }

        int total_stack_size = (int)stack.size();
        int arg_start_index = total_stack_size - argc;

        Frame frame;
        frame.return_pc = pc + 1;
        frame.prev_stack_size = arg_start_index;
        frame.arg_count = argc;

        if (runtime.isBuiltin(label)) {
            std::vector<int> args;
            for (int i = 0; i < argc; i++) {
                int arg = pop();
                args.insert(args.begin(), arg);
            }

            int result = runtime.call(label, args);
            push(result);
            pc++;
        } else {
            if (callstack.size() >= maxCallDepth_) {
                std::cerr << "ERROR: Call stack overflow (max depth " << maxCallDepth_ << " exceeded at PC=" << pc << ")" << std::endl;
                pc = (int)program.size();
                return;
            }
            callstack.push_back(frame);

            if (labels.find(label) == labels.end()) {
                std::cerr << "ERROR: Label '" << label << "' not found at PC=" << pc << std::endl;
                pc++;
            } else {
                pc = labels[label];
            }
        }
    }

    else if (opcode == "LOADARG") {
        if (parts.size() < 2) {
            std::cerr << "ERROR: LOADARG requires argument index at PC=" << pc << std::endl;
            push(0);
            pc++;
            return;
        }

        int arg_index = std::stoi(parts[1]);

        if (callstack.empty()) {
            std::cerr << "WARNING: LOADARG called outside function at PC=" << pc << std::endl;
            push(0);
            pc++;
            return;
        }

        Frame& frame = callstack.back();

        if (arg_index < 0 || arg_index >= frame.arg_count) {
            std::cerr << "WARNING: Invalid argument index " << arg_index << " at PC=" << pc << std::endl;
            push(0);
            pc++;
            return;
        }

        int arg_position = frame.prev_stack_size + arg_index;

        if (arg_position < (int)stack.size()) {
            push(stack[arg_position]);
        } else {
            std::cerr << "WARNING: Argument position out of bounds at PC=" << pc << std::endl;
            push(0);
        }
        pc++;
    }

    else if (opcode == "RET") {
        if (callstack.empty()) {
            pc = (int)program.size();
            return;
        }

        int return_value = 0;
        if (!stack.empty()) {
            return_value = pop();
        }

        Frame frame = callstack.back();
        callstack.pop_back();

        // Drop locals/temporaries pushed during the call
        stack.resize(frame.prev_stack_size);
        push(return_value);
        pc = frame.return_pc;
    }

    else if (opcode == "PRINT") {
        if (stack.empty()) {
            std::cout << "[EMPTY_STACK]" << std::endl;
        } else {
            std::cout << stack.back() << std::endl;
        }
        pc++;
    }

    else {
        std::cerr << "WARNING: Unknown instruction '" << opcode << "' at PC=" << pc << std::endl;
        pc++;
    }
}

void VM::run(std::vector<std::string> program, size_t maxSteps, size_t maxCallDepth) {
    preprocess(program);

    pc = 0;
    stack.clear();
    vars.clear();
    callstack.clear();
    maxCallDepth_ = maxCallDepth;

    size_t steps = 0;
    while (pc < (int)program.size()) {
        if (maxSteps > 0 && steps >= maxSteps) {
            std::cerr << "ERROR: Execution limit exceeded (" << maxSteps
                      << " steps - possible infinite loop)" << std::endl;
            return;
        }
        steps++;
        execute(program[pc], program);
    }
}

void VM::printStack() const {
    std::cout << "Stack: [";
    for (size_t i = 0; i < stack.size(); i++) {
        std::cout << stack[i];
        if (i < stack.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void VM::printVars() const {
    std::cout << "Variables:" << std::endl;
    for (const auto& pair : vars) {
        std::cout << "  " << pair.first << " = " << pair.second << std::endl;
    }
}

void VM::reset() {
    pc = 0;
    stack.clear();
    vars.clear();
    labels.clear();
    callstack.clear();
}
