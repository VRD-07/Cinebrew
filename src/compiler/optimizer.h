#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "ast.h"
#include <memory>

class Optimizer {
public:
    void optimize(std::unique_ptr<Program>& program);

private:
    void optimizeStmt(Stmt* stmt);
    void optimizeBlock(BlockStmt* block);
    std::unique_ptr<Expr> optimizeExpr(std::unique_ptr<Expr> expr);
};

#endif
