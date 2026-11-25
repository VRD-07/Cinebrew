#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    TAKE,
    POUR,
    SCENE,
    SHOT,
    IF,
    ELSE,
    LOOP,
    BREAK,
    CONTINUE,
    TRUE_KW,
    FALSE_KW,

    IDENTIFIER,
    NUMBER,
    STRING,

    EQUAL,
    PLUS,
    MINUS,
    STAR,
    SLASH,

    EQUAL_EQUAL,
    BANG_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,

    SEMICOLON,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    COMMA,

    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::string literal;
    int line;

    Token(TokenType t, std::string lex, std::string lit, int l)
        : type(t), lexeme(lex), literal(lit), line(l) {}

    Token(TokenType t, std::string lex, int l)
        : type(t), lexeme(lex), literal(""), line(l) {}
};

std::string tokenTypeToString(TokenType type);

#endif
