#include "optimizer.h"
#include <string>

static bool isNumberLiteral(const Expr* expr, int& outValue) {
    const LiteralExpr* lit = dynamic_cast<const LiteralExpr*>(expr);
    if (!lit || lit->token.type != TokenType::NUMBER) {
        return false;
    }
    try {
        outValue = std::stoi(lit->value);
        return true;
    } catch (...) {
        return false;
    }
}

static std::unique_ptr<LiteralExpr> makeNumberLiteral(int value, int line) {
    std::string text = std::to_string(value);
    Token tok(TokenType::NUMBER, text, text, line);
    return std::make_unique<LiteralExpr>(tok, text);
}

void Optimizer::optimize(std::unique_ptr<Program>& program) {
    if (!program) return;
    for (auto& stmt : program->statements) {
        optimizeStmt(stmt.get());
    }
}

void Optimizer::optimizeBlock(BlockStmt* block) {
    if (!block) return;
    for (auto& stmt : block->statements) {
        optimizeStmt(stmt.get());
    }
}

void Optimizer::optimizeStmt(Stmt* stmt) {
    if (!stmt) return;

    if (DeclarationStmt* decl = dynamic_cast<DeclarationStmt*>(stmt)) {
        decl->initializer = optimizeExpr(std::move(decl->initializer));
    } else if (AssignmentStmt* assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        assign->value = optimizeExpr(std::move(assign->value));
    } else if (ExpressionStmt* exprStmt = dynamic_cast<ExpressionStmt*>(stmt)) {
        exprStmt->expression = optimizeExpr(std::move(exprStmt->expression));
    } else if (PrintStmt* printStmt = dynamic_cast<PrintStmt*>(stmt)) {
        printStmt->expression = optimizeExpr(std::move(printStmt->expression));
    } else if (IfStmt* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        ifStmt->condition = optimizeExpr(std::move(ifStmt->condition));
        optimizeBlock(ifStmt->thenBranch.get());
        if (ifStmt->elseBranch) {
            optimizeBlock(ifStmt->elseBranch.get());
        }
    } else if (LoopStmt* loopStmt = dynamic_cast<LoopStmt*>(stmt)) {
        loopStmt->condition = optimizeExpr(std::move(loopStmt->condition));
        optimizeBlock(loopStmt->body.get());
    } else if (ReturnStmt* retStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        if (retStmt->value) {
            retStmt->value = optimizeExpr(std::move(retStmt->value));
        }
    } else if (FunctionStmt* funcStmt = dynamic_cast<FunctionStmt*>(stmt)) {
        optimizeBlock(funcStmt->body.get());
    } else if (BlockStmt* blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        optimizeBlock(blockStmt);
    }
}

std::unique_ptr<Expr> Optimizer::optimizeExpr(std::unique_ptr<Expr> expr) {
    if (!expr) return nullptr;

    if (BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr.get())) {
        // Optimize children first so folded constants unlock parent rewrite rules
        bin->left = optimizeExpr(std::move(bin->left));
        bin->right = optimizeExpr(std::move(bin->right));

        int leftVal = 0;
        int rightVal = 0;
        bool hasLeft = isNumberLiteral(bin->left.get(), leftVal);
        bool hasRight = isNumberLiteral(bin->right.get(), rightVal);
        int line = bin->op.line;

        if (hasLeft && hasRight) {
            switch (bin->op.type) {
                case TokenType::PLUS:          return makeNumberLiteral(leftVal + rightVal, line);
                case TokenType::MINUS:         return makeNumberLiteral(leftVal - rightVal, line);
                case TokenType::STAR:          return makeNumberLiteral(leftVal * rightVal, line);
                case TokenType::SLASH:
                    if (rightVal != 0)     return makeNumberLiteral(leftVal / rightVal, line);
                    break;
                case TokenType::EQUAL_EQUAL:   return makeNumberLiteral(leftVal == rightVal ? 1 : 0, line);
                case TokenType::BANG_EQUAL:    return makeNumberLiteral(leftVal != rightVal ? 1 : 0, line);
                case TokenType::GREATER:       return makeNumberLiteral(leftVal > rightVal ? 1 : 0, line);
                case TokenType::LESS:          return makeNumberLiteral(leftVal < rightVal ? 1 : 0, line);
                case TokenType::GREATER_EQUAL: return makeNumberLiteral(leftVal >= rightVal ? 1 : 0, line);
                case TokenType::LESS_EQUAL:    return makeNumberLiteral(leftVal <= rightVal ? 1 : 0, line);
                default: break;
            }
        }

        // Algebraic identity & zero rules
        if (bin->op.type == TokenType::PLUS) {
            if (hasRight && rightVal == 0) return std::move(bin->left);
            if (hasLeft && leftVal == 0)   return std::move(bin->right);
        } else if (bin->op.type == TokenType::MINUS) {
            if (hasRight && rightVal == 0) return std::move(bin->left);
        } else if (bin->op.type == TokenType::STAR) {
            if ((hasRight && rightVal == 0) || (hasLeft && leftVal == 0)) {
                return makeNumberLiteral(0, line);
            }
            if (hasRight && rightVal == 1) return std::move(bin->left);
            if (hasLeft && leftVal == 1)   return std::move(bin->right);
        } else if (bin->op.type == TokenType::SLASH) {
            if (hasRight && rightVal == 1) return std::move(bin->left);
        }

        return expr;
    }

    if (UnaryExpr* un = dynamic_cast<UnaryExpr*>(expr.get())) {
        un->right = optimizeExpr(std::move(un->right));
        int val = 0;
        if (un->op.type == TokenType::MINUS && isNumberLiteral(un->right.get(), val)) {
            return makeNumberLiteral(-val, un->op.line);
        }
        return expr;
    }

    if (CallExpr* call = dynamic_cast<CallExpr*>(expr.get())) {
        for (auto& arg : call->arguments) {
            arg = optimizeExpr(std::move(arg));
        }
        return expr;
    }

    return expr;
}
