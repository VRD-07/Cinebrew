#include "semantic.h"
#include "../runtime/runtime.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer() : hadError_(false) {
    runtime_ = std::make_unique<Runtime>();
}

void SemanticAnalyzer::error(const Token& token, const std::string& message) {
    hadError_ = true;
    std::string errorMsg = "Line " + std::to_string(token.line) + ": " + message;
    errors_.push_back(errorMsg);
    std::cerr << "Semantic Error: " << errorMsg << std::endl;
}

void SemanticAnalyzer::declare(const Token& name, SymbolType type, int paramCount) {
    std::string nameStr = name.lexeme;

    if (symbols_.find(nameStr) != symbols_.end()) {
        Symbol& existing = symbols_[nameStr];
        error(name, "Redeclaration of '" + nameStr + "' (first declared at line " +
              std::to_string(existing.line) + ")");
        return;
    }

    symbols_[nameStr] = Symbol(type, nameStr, name.line, paramCount);
}

Symbol* SemanticAnalyzer::resolve(const Token& name, SymbolType expectedType) {
    std::string nameStr = name.lexeme;

    auto it = symbols_.find(nameStr);
    if (it == symbols_.end()) {
        std::string typeStr = (expectedType == SymbolType::VARIABLE ? "variable" : "function");
        error(name, "Undefined " + typeStr + ": '" + nameStr + "'");
        return nullptr;
    }

    Symbol& symbol = it->second;
    if (symbol.type != expectedType) {
        error(name, "'" + nameStr + "' is a " +
              (symbol.type == SymbolType::VARIABLE ? "variable" : "function") +
              ", not a " +
              (expectedType == SymbolType::VARIABLE ? "variable" : "function"));
        return nullptr;
    }

    return &symbol;
}

void SemanticAnalyzer::analyze(std::unique_ptr<Program>& program) {
    symbols_.clear();
    errors_.clear();
    hadError_ = false;

    visitProgram(program.get());
}

void SemanticAnalyzer::visitProgram(Program* program) {
    // Hoist all function declarations so calls can precede definitions
    for (auto& stmt : program->statements) {
        if (FunctionStmt* func = dynamic_cast<FunctionStmt*>(stmt.get())) {
            declare(func->name, SymbolType::FUNCTION, func->parameters.size());
        }
    }

    for (auto& stmt : program->statements) {
        visitStmt(stmt.get());
    }
}

void SemanticAnalyzer::visitStmt(Stmt* stmt) {
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

void SemanticAnalyzer::visitDeclaration(DeclarationStmt* stmt) {
    visitExpr(stmt->initializer.get());
    declare(stmt->name, SymbolType::VARIABLE);
}

void SemanticAnalyzer::visitAssignment(AssignmentStmt* stmt) {
    resolve(stmt->name, SymbolType::VARIABLE);
    visitExpr(stmt->value.get());
}

void SemanticAnalyzer::visitPrint(PrintStmt* stmt) {
    visitExpr(stmt->expression.get());
}

void SemanticAnalyzer::visitIf(IfStmt* stmt) {
    visitExpr(stmt->condition.get());
    visitBlock(stmt->thenBranch.get());
    if (stmt->elseBranch) {
        visitBlock(stmt->elseBranch.get());
    }
}

void SemanticAnalyzer::visitLoop(LoopStmt* stmt) {
    visitExpr(stmt->condition.get());
    visitBlock(stmt->body.get());
}

void SemanticAnalyzer::visitBreak(BreakStmt* stmt) {}

void SemanticAnalyzer::visitContinue(ContinueStmt* stmt) {}

void SemanticAnalyzer::visitReturn(ReturnStmt* stmt) {
    if (stmt->value) {
        visitExpr(stmt->value.get());
    }
}

void SemanticAnalyzer::visitFunction(FunctionStmt* stmt) {
    // Temporarily shadow outer symbols with function parameters during body analysis
    std::unordered_map<std::string, bool> previousExisted;
    std::unordered_map<std::string, Symbol> previousSymbols;

    for (const Token& param : stmt->parameters) {
        std::string name = param.lexeme;
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            previousExisted[name] = true;
            previousSymbols[name] = it->second;
        } else {
            previousExisted[name] = false;
        }

        symbols_[name] = Symbol(SymbolType::VARIABLE, name, param.line);
    }

    visitBlock(stmt->body.get());

    for (const Token& param : stmt->parameters) {
        std::string name = param.lexeme;
        if (previousExisted[name]) {
            symbols_[name] = previousSymbols[name];
        } else {
            symbols_.erase(name);
        }
    }
}

void SemanticAnalyzer::visitBlock(BlockStmt* stmt) {
    for (auto& stmt : stmt->statements) {
        visitStmt(stmt.get());
    }
}

void SemanticAnalyzer::visitExpr(Expr* expr) {
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

void SemanticAnalyzer::visitLiteral(LiteralExpr* expr) {}

void SemanticAnalyzer::visitVariable(VariableExpr* expr) {
    resolve(expr->name, SymbolType::VARIABLE);
}

void SemanticAnalyzer::visitBinary(BinaryExpr* expr) {
    visitExpr(expr->left.get());
    visitExpr(expr->right.get());
}

void SemanticAnalyzer::visitUnary(UnaryExpr* expr) {
    visitExpr(expr->right.get());
}

void SemanticAnalyzer::visitCall(CallExpr* expr) {
    std::string funcName = expr->callee.lexeme;

    if (runtime_->isBuiltin(funcName)) {
        int expectedArgs = runtime_->getParamCount(funcName);
        int actualArgs = expr->arguments.size();

        if (actualArgs != expectedArgs) {
            error(expr->callee, "Built-in function '" + funcName + "' expects " +
                  std::to_string(expectedArgs) + " argument(s), but got " +
                  std::to_string(actualArgs));
        }
    } else {
        Symbol* symbol = resolve(expr->callee, SymbolType::FUNCTION);

        if (symbol) {
            int expectedArgs = symbol->paramCount;
            int actualArgs = expr->arguments.size();

            if (actualArgs != expectedArgs) {
                error(expr->callee, "Function '" + funcName + "' expects " +
                      std::to_string(expectedArgs) + " argument(s), but got " +
                      std::to_string(actualArgs));
            }
        }
    }

    for (auto& arg : expr->arguments) {
        visitExpr(arg.get());
    }
}
