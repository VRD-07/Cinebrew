#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "semantic.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class CodeGenerator {
public:
    CodeGenerator();

    std::vector<std::string> generate(std::unique_ptr<Program>& program);
    std::vector<std::string> getBytecode() const { return bytecode_; }
    bool hadError() const { return hadError_; }
    std::string getError() const { return errorMessage_; }

private:
    std::vector<std::string> bytecode_;
    std::unordered_map<std::string, int> labelCounter_;
    std::unordered_map<std::string, int> paramMap_;
    bool hadError_;
    std::string errorMessage_;

    void emit(const std::string& instruction);
    std::string newLabel(const std::string& prefix);
    void error(const std::string& message);

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
