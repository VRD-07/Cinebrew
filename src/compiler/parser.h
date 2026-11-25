#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parse();
    bool hadError() const { return hadError_; }
    std::string getError() const { return errorMessage_; }

private:
    std::vector<Token> tokens_;
    int current_;
    bool hadError_;
    std::string errorMessage_;

    bool isAtEnd() const;
    Token peek() const;
    Token peekNext() const;
    Token previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);

    void error(const Token& token, const std::string& message);
    void synchronize();

    std::unique_ptr<Program> program();
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> statement();

    std::unique_ptr<Stmt> declarationStmt();
    std::unique_ptr<Stmt> assignmentStmt();
    std::unique_ptr<Stmt> printStmt();
    std::unique_ptr<Stmt> ifStmt();
    std::unique_ptr<Stmt> loopStmt();
    std::unique_ptr<Stmt> breakStmt();
    std::unique_ptr<Stmt> continueStmt();
    std::unique_ptr<Stmt> returnStmt();
    std::unique_ptr<Stmt> functionStmt();
    std::unique_ptr<BlockStmt> block();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> multiplication();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> finishCall(const Token& callee);
};

#endif
