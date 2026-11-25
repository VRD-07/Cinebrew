#include "codegen.h"
#include <sstream>
#include <iostream>

CodeGenerator::CodeGenerator() : hadError_(false) {}

void CodeGenerator::error(const std::string& message) {
    hadError_ = true;
    errorMessage_ = message;
    std::cerr << "CodeGen Error: " << message << std::endl;
}

void CodeGenerator::emit(const std::string& instruction) {
    bytecode_.push_back(instruction);
}

std::string CodeGenerator::newLabel(const std::string& prefix) {
    int count = labelCounter_[prefix]++;
    return prefix + "_" + std::to_string(count);
}

std::vector<std::string> CodeGenerator::generate(std::unique_ptr<Program>& program) {
    bytecode_.clear();
    labelCounter_.clear();
    hadError_ = false;
    errorMessage_ = "";

    visitProgram(program.get());

    return bytecode_;
}

void CodeGenerator::visitProgram(Program* program) {
    std::string startLabel = newLabel("main_entry");
    emit("JMP " + startLabel);

    std::vector<FunctionStmt*> functions;
    std::vector<Stmt*> mainStmts;

    for (auto& stmt : program->statements) {
        if (FunctionStmt* func = dynamic_cast<FunctionStmt*>(stmt.get())) {
            functions.push_back(func);
        } else {
            mainStmts.push_back(stmt.get());
        }
    }

    for (FunctionStmt* func : functions) {
        visitFunction(func);
    }

    emit(startLabel + ":");
    for (Stmt* stmt : mainStmts) {
        visitStmt(stmt);
    }

    emit("HALT:");
}

void CodeGenerator::visitStmt(Stmt* stmt) {
    if (DeclarationStmt* decl = dynamic_cast<DeclarationStmt*>(stmt)) {
        visitDeclaration(decl);
    } else if (AssignmentStmt* assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        visitAssignment(assign);
    } else if (PrintStmt* print = dynamic_cast<PrintStmt*>(stmt)) {
        visitPrint(print);
    } else if (IfStmt* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        visitIf(ifStmt);
    } else if (LoopStmt* loop = dynamic_cast<LoopStmt*>(stmt)) {
        visitLoop(loop);
    } else if (BreakStmt* brk = dynamic_cast<BreakStmt*>(stmt)) {
        visitBreak(brk);
    } else if (ContinueStmt* cont = dynamic_cast<ContinueStmt*>(stmt)) {
        visitContinue(cont);
    } else if (ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt)) {
        visitReturn(ret);
    } else if (FunctionStmt* func = dynamic_cast<FunctionStmt*>(stmt)) {
        visitFunction(func);
    } else if (BlockStmt* block = dynamic_cast<BlockStmt*>(stmt)) {
        visitBlock(block);
    } else if (ExpressionStmt* expr = dynamic_cast<ExpressionStmt*>(stmt)) {
        visitExpr(expr->expression.get());
    }
}

void CodeGenerator::visitDeclaration(DeclarationStmt* stmt) {
    visitExpr(stmt->initializer.get());
    emit("STORE " + stmt->name.lexeme);
}

void CodeGenerator::visitAssignment(AssignmentStmt* stmt) {
    visitExpr(stmt->value.get());
    emit("STORE " + stmt->name.lexeme);
}

void CodeGenerator::visitPrint(PrintStmt* stmt) {
    visitExpr(stmt->expression.get());
    emit("PRINT");
}

void CodeGenerator::visitIf(IfStmt* stmt) {
    visitExpr(stmt->condition.get());

    std::string elseLabel = newLabel("else");
    std::string endLabel = newLabel("end_if");

    emit("JZ " + elseLabel);
    visitBlock(stmt->thenBranch.get());
    emit("JMP " + endLabel);

    emit(elseLabel + ":");
    if (stmt->elseBranch) {
        visitBlock(stmt->elseBranch.get());
    }

    emit(endLabel + ":");
}

void CodeGenerator::visitLoop(LoopStmt* stmt) {
    std::string loopLabel = newLabel("loop");
    std::string endLabel = newLabel("end_loop");

    emit(loopLabel + ":");
    visitExpr(stmt->condition.get());
    emit("JZ " + endLabel);
    visitBlock(stmt->body.get());
    emit("JMP " + loopLabel);
    emit(endLabel + ":");
}

void CodeGenerator::visitBreak(BreakStmt* stmt) {
    // TODO: bind to enclosing loop's end label
    std::string endLabel = newLabel("end_loop");
    emit("JMP " + endLabel);
}

void CodeGenerator::visitContinue(ContinueStmt* stmt) {
    // TODO: bind to enclosing loop's start label
    std::string loopLabel = newLabel("loop");
    emit("JMP " + loopLabel);
}

void CodeGenerator::visitReturn(ReturnStmt* stmt) {
    if (stmt->value) {
        visitExpr(stmt->value.get());
    } else {
        emit("PUSH 0");
    }
    emit("RET");
}

void CodeGenerator::visitFunction(FunctionStmt* stmt) {
    emit(stmt->name.lexeme + ":");

    paramMap_.clear();
    for (size_t i = 0; i < stmt->parameters.size(); ++i) {
        paramMap_[stmt->parameters[i].lexeme] = (int)i;
    }

    visitBlock(stmt->body.get());
    paramMap_.clear();

    // Fallback return if body has no explicit SHOT
    emit("PUSH 0");
    emit("RET");
}

void CodeGenerator::visitBlock(BlockStmt* stmt) {
    for (auto& stmt : stmt->statements) {
        visitStmt(stmt.get());
    }
}

void CodeGenerator::visitExpr(Expr* expr) {
    if (LiteralExpr* lit = dynamic_cast<LiteralExpr*>(expr)) {
        visitLiteral(lit);
    } else if (VariableExpr* var = dynamic_cast<VariableExpr*>(expr)) {
        visitVariable(var);
    } else if (BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr)) {
        visitBinary(bin);
    } else if (UnaryExpr* un = dynamic_cast<UnaryExpr*>(expr)) {
        visitUnary(un);
    } else if (CallExpr* call = dynamic_cast<CallExpr*>(expr)) {
        visitCall(call);
    }
}

void CodeGenerator::visitLiteral(LiteralExpr* expr) {
    if (expr->token.type == TokenType::STRING) {
        emit("PUSH \"" + expr->value + "\"");
    } else {
        emit("PUSH " + expr->value);
    }
}

void CodeGenerator::visitVariable(VariableExpr* expr) {
    auto it = paramMap_.find(expr->name.lexeme);
    if (it != paramMap_.end()) {
        emit("LOADARG " + std::to_string(it->second));
    } else {
        emit("LOAD " + expr->name.lexeme);
    }
}

void CodeGenerator::visitBinary(BinaryExpr* expr) {
    visitExpr(expr->left.get());
    visitExpr(expr->right.get());

    std::string op = expr->op.lexeme;

    if (op == "+") {
        emit("ADD");
    } else if (op == "-") {
        emit("SUB");
    } else if (op == "*") {
        emit("MUL");
    } else if (op == "/") {
        emit("DIV");
    } else if (op == "==") {
        emit("EQ");
    } else if (op == "!=") {
        // 1 - (a == b)
        emit("EQ");
        emit("PUSH 1");
        emit("SUB");
    } else if (op == ">") {
        emit("GT");
    } else if (op == "<") {
        emit("LT");
    } else if (op == ">=") {
        // 1 - (a < b)
        emit("LT");
        emit("PUSH 1");
        emit("SUB");
    } else if (op == "<=") {
        // 1 - (a > b)
        emit("GT");
        emit("PUSH 1");
        emit("SUB");
    } else {
        error("Unknown binary operator: " + op);
    }
}

void CodeGenerator::visitUnary(UnaryExpr* expr) {
    visitExpr(expr->right.get());

    std::string op = expr->op.lexeme;

    if (op == "-") {
        emit("PUSH 0");
        emit("SUB");
    } else if (op == "!") {
        emit("PUSH 1");
        emit("SUB");
    } else {
        error("Unknown unary operator: " + op);
    }
}

void CodeGenerator::visitCall(CallExpr* expr) {
    for (auto& arg : expr->arguments) {
        visitExpr(arg.get());
    }

    int argCount = expr->arguments.size();
    emit("CALL " + expr->callee.lexeme + " " + std::to_string(argCount));
}
