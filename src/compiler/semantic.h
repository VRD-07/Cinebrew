#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "token.h"
#include "../runtime/runtime.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

enum class SymbolType {
    VARIABLE,
    FUNCTION
};

struct Symbol {
    SymbolType type;
    std::string name;
    int line;
    int paramCount;

    Symbol() : type(SymbolType::VARIABLE), name(""), line(0), paramCount(0) {}

    Symbol(SymbolType t, const std::string& n, int l, int params = 0)
        : type(t), name(n), line(l), paramCount(params) {}
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    void analyze(std::unique_ptr<Program>& program);
    bool hadError() const { return hadError_; }
    std::vector<std::string> getErrors() const { return errors_; }

private:
    std::unordered_map<std::string, Symbol> symbols_;
    std::vector<std::string> errors_;
    bool hadError_;
    std::unique_ptr<Runtime> runtime_;

    void error(const Token& token, const std::string& message);
    void declare(const Token& name, SymbolType type, int paramCount = 0);
    Symbol* resolve(const Token& name, SymbolType expectedType);

    void visitProgram(Program* program);
    void visitStmt(Stmt* stmt);
    void visitExpr(Expr* expr);

    void visitDeclaration(DeclarationStmt* stmt);
    void visitAssignment(AssignmentStmt* stmt);
    void visitPrint(PrintStmt* stmt);
    void visitIf(IfStmt* stmt);
    void visitLoop(LoopStmt* stmt);
    void visitBreak(BreakStmt* stmt);
    void visitContinue(ContinueStmt* stmt);
    void visitReturn(ReturnStmt* stmt);
    void visitFunction(FunctionStmt* stmt);
    void visitBlock(BlockStmt* stmt);

    void visitLiteral(LiteralExpr* expr);
    void visitVariable(VariableExpr* expr);
    void visitBinary(BinaryExpr* expr);
    void visitUnary(UnaryExpr* expr);
    void visitCall(CallExpr* expr);
};

#endif
