#ifndef AST_H
#define AST_H

#include "token.h"
#include <vector>
#include <string>
#include <memory>

class Expr;
class Stmt;
class BlockStmt;

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string toString() const = 0;
};

class LiteralExpr : public Expr {
public:
    Token token;
    std::string value;

    LiteralExpr(const Token& tok, const std::string& val)
        : token(tok), value(val) {}

    std::string toString() const override;
};

class VariableExpr : public Expr {
public:
    Token name;

    VariableExpr(const Token& tok) : name(tok) {}

    std::string toString() const override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, const Token& o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}

    std::string toString() const override;
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(const Token& o, std::unique_ptr<Expr> r)
        : op(o), right(std::move(r)) {}

    std::string toString() const override;
};

class CallExpr : public Expr {
public:
    Token callee;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(const Token& tok, std::vector<std::unique_ptr<Expr>> args)
        : callee(tok), arguments(std::move(args)) {}

    std::string toString() const override;
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual std::string toString() const = 0;
};

class DeclarationStmt : public Stmt {
public:
    Token keyword;
    Token name;
    std::unique_ptr<Expr> initializer;

    DeclarationStmt(const Token& kw, const Token& n, std::unique_ptr<Expr> init)
        : keyword(kw), name(n), initializer(std::move(init)) {}

    std::string toString() const override;
};

class AssignmentStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> value;

    AssignmentStmt(const Token& n, std::unique_ptr<Expr> val)
        : name(n), value(std::move(val)) {}

    std::string toString() const override;
};

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;

    ExpressionStmt(std::unique_ptr<Expr> expr)
        : expression(std::move(expr)) {}

    std::string toString() const override;
};

class PrintStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> expression;

    PrintStmt(const Token& kw, std::unique_ptr<Expr> expr)
        : keyword(kw), expression(std::move(expr)) {}

    std::string toString() const override;
};

class IfStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBranch;
    std::unique_ptr<BlockStmt> elseBranch;

    IfStmt(const Token& kw, std::unique_ptr<Expr> cond,
           std::unique_ptr<BlockStmt> thenBr,
           std::unique_ptr<BlockStmt> elseBr)
        : keyword(kw), condition(std::move(cond)),
          thenBranch(std::move(thenBr)), elseBranch(std::move(elseBr)) {}

    std::string toString() const override;
};

class LoopStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;

    LoopStmt(const Token& kw, std::unique_ptr<Expr> cond,
             std::unique_ptr<BlockStmt> b)
        : keyword(kw), condition(std::move(cond)), body(std::move(b)) {}

    std::string toString() const override;
};

class BreakStmt : public Stmt {
public:
    Token keyword;

    BreakStmt(const Token& kw) : keyword(kw) {}

    std::string toString() const override;
};

class ContinueStmt : public Stmt {
public:
    Token keyword;

    ContinueStmt(const Token& kw) : keyword(kw) {}

    std::string toString() const override;
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;

    ReturnStmt(const Token& kw, std::unique_ptr<Expr> val)
        : keyword(kw), value(std::move(val)) {}

    std::string toString() const override;
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}

    std::string toString() const override;
};

class FunctionStmt : public Stmt {
public:
    Token keyword;
    Token name;
    std::vector<Token> parameters;
    std::unique_ptr<BlockStmt> body;

    FunctionStmt(const Token& kw, const Token& n,
                 std::vector<Token> params, std::unique_ptr<BlockStmt> b)
        : keyword(kw), name(n), parameters(std::move(params)), body(std::move(b)) {}

    std::string toString() const override;
};

class Program {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    Program(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}

    std::string toString() const;
};

#endif
