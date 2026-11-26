#ifndef COMPILER_H
#define COMPILER_H

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "optimizer.h"
#include "codegen.h"
#include <string>
#include <vector>
#include <memory>

class Compiler {
public:
    Compiler();

    std::vector<std::string> compile(const std::string& source, bool optimize = true);
    bool hadError() const;
    std::vector<std::string> getErrors() const;
    std::vector<std::string> getBytecode() const;

private:
    std::vector<std::string> errors_;
    std::vector<std::string> bytecode_;
    bool hadError_;
};

#endif
