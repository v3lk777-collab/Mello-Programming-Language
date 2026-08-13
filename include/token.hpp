// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once

#include <string>

enum class TokenType {
    KEYWORD,
    SERIAL_KEYWORD,
    SYMBOL,
    NUMBER,
    STRING,
    EQUAL,
    EQUALITY,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    NOT_EQUAL,
    PLUS_EQUAL,
    MINUS_EQUAL,
    PLUS_PLUS,
    MINUS_MINUS,
    BANG,
    MINUS,
    PLUS,
    MUL,
    DIV,
    DOT,
    COLON,
    COMMA,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    INDENT,
    DEDENT,
    NEWLINE,
    EndOfFile
};

struct Token {
    int line;
    int column;
    TokenType type;
    std::string value;

    Token()
        : type(TokenType::EndOfFile), value(""), line(0), column(0) {}

    Token(TokenType type, const std::string& value, int line = 0, int column = 0)
        : type(type), value(value), line(line), column(column) {}
};