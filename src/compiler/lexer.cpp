#include "lexer.h"
#include <iostream>
#include <cctype>

std::unordered_map<std::string, TokenType> Lexer::keywords_;

void Lexer::initializeKeywords() {
    if (!keywords_.empty()) return;

    keywords_["TAKE"] = TokenType::TAKE;
    keywords_["POUR"] = TokenType::POUR;
    keywords_["SCENE"] = TokenType::SCENE;
    keywords_["SHOT"] = TokenType::SHOT;
    keywords_["IF"] = TokenType::IF;
    keywords_["ELSE"] = TokenType::ELSE;
    keywords_["LOOP"] = TokenType::LOOP;
    keywords_["BREAK"] = TokenType::BREAK;
    keywords_["CONTINUE"] = TokenType::CONTINUE;
    keywords_["true"] = TokenType::TRUE_KW;
    keywords_["false"] = TokenType::FALSE_KW;
}

Lexer::Lexer(const std::string& source)
    : source_(source), start_(0), current_(0), line_(1), hadError_(false) {
    initializeKeywords();
}

std::vector<Token> Lexer::tokenize() {
    tokens_.clear();
    start_ = 0;
    current_ = 0;
    line_ = 1;
    hadError_ = false;
    errorMessage_ = "";

    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }

    tokens_.push_back(Token(TokenType::END_OF_FILE, "", line_));
    return tokens_;
}

bool Lexer::isAtEnd() const {
    return current_ >= (int)source_.length();
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= (int)source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    return true;
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

void Lexer::addToken(TokenType type) {
    std::string lexeme = getCurrentLexeme();
    tokens_.push_back(Token(type, lexeme, line_));
}

void Lexer::addToken(TokenType type, const std::string& literal) {
    std::string lexeme = getCurrentLexeme();
    tokens_.push_back(Token(type, lexeme, literal, line_));
}

void Lexer::error(const std::string& message) {
    hadError_ = true;
    errorMessage_ = "Line " + std::to_string(line_) + ": " + message;
    std::cerr << "Lexer Error: " << errorMessage_ << std::endl;
}

std::string Lexer::getCurrentLexeme() const {
    return source_.substr(start_, current_ - start_);
}

void Lexer::scanToken() {
    char c = advance();

    switch (c) {
        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line_++;
            break;

        case '#':
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
            break;

        case ';':
            addToken(TokenType::SEMICOLON);
            break;

        case '(':
            addToken(TokenType::LPAREN);
            break;

        case ')':
            addToken(TokenType::RPAREN);
            break;

        case '{':
            addToken(TokenType::LBRACE);
            break;

        case '}':
            addToken(TokenType::RBRACE);
            break;

        case ',':
            addToken(TokenType::COMMA);
            break;

        case '+':
            addToken(TokenType::PLUS);
            break;

        case '-':
            if (isDigit(peek())) {
                scanNumber();
            } else {
                addToken(TokenType::MINUS);
            }
            break;

        case '*':
            addToken(TokenType::STAR);
            break;

        case '/':
            addToken(TokenType::SLASH);
            break;

        case '=':
            if (match('=')) {
                addToken(TokenType::EQUAL_EQUAL);
            } else {
                addToken(TokenType::EQUAL);
            }
            break;

        case '!':
            if (match('=')) {
                addToken(TokenType::BANG_EQUAL);
            } else {
                error("Unexpected character '!' (did you mean '!='?)");
                addToken(TokenType::ERROR);
            }
            break;

        case '>':
            if (match('=')) {
                addToken(TokenType::GREATER_EQUAL);
            } else {
                addToken(TokenType::GREATER);
            }
            break;

        case '<':
            if (match('=')) {
                addToken(TokenType::LESS_EQUAL);
            } else {
                addToken(TokenType::LESS);
            }
            break;

        case '"':
            scanString();
            break;

        default:
            if (isDigit(c)) {
                scanNumber();
            } else if (isAlpha(c)) {
                scanIdentifier();
            } else {
                error("Unexpected character: '" + std::string(1, c) + "'");
                addToken(TokenType::ERROR);
            }
            break;
    }
}

void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line_++;
        }
        advance();
    }

    if (isAtEnd()) {
        error("Unterminated string literal");
        return;
    }

    advance();

    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::scanNumber() {
    while (isDigit(peek())) {
        advance();
    }

    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) {
            advance();
        }
        error("Floating point numbers not yet supported");
    }

    std::string value = getCurrentLexeme();
    addToken(TokenType::NUMBER, value);
}

void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) {
        advance();
    }

    std::string text = getCurrentLexeme();

    if (isDigit(text[0])) {
        error("Identifier cannot start with a digit: " + text);
        addToken(TokenType::ERROR);
        return;
    }

    auto it = keywords_.find(text);
    if (it != keywords_.end()) {
        addToken(it->second);
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}
