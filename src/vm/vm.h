#ifndef VM_H
#define VM_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include "../runtime/runtime.h"

struct Frame {
    int return_pc;
    int prev_stack_size;
    int arg_count;
};

class VM {
public:
    std::vector<int> stack;
    std::unordered_map<std::string, int> vars;
    int pc;
    std::unordered_map<std::string, int> labels;
    std::vector<Frame> callstack;
    Runtime runtime;

    VM();

    void push(int value);
    int pop();

    void preprocess(std::vector<std::string>& program);
    void execute(const std::string& instruction, std::vector<std::string>& program);
    void run(std::vector<std::string> program, size_t maxSteps = 0, size_t maxCallDepth = 512);

    void printStack() const;
    void printVars() const;
    void reset();

private:
    size_t maxCallDepth_ = 512;
    std::vector<std::string> split(const std::string& s);
};

#endif
