#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <vector>
#include <string>
#include <unordered_map>

class Lexer {
public:
    Lexer(const std::string& source);

    std::vector<Token> tokenize();
    bool hadError() const { return hadError_; }
    std::string getError() const { return errorMessage_; }

private:
    std::string source_;
    std::vector<Token> tokens_;
    int start_;
    int current_;
    int line_;
    bool hadError_;
    std::string errorMessage_;

    static std::unordered_map<std::string, TokenType> keywords_;
    static void initializeKeywords();

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;

    void addToken(TokenType type);
    void addToken(TokenType type, const std::string& literal);
    void error(const std::string& message);

    void scanToken();
    void scanString();
    void scanNumber();
    void scanIdentifier();
    void scanOperator();

    std::string getCurrentLexeme() const;
};

#endif
