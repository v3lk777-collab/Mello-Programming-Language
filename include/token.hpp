/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
*/

#pragma once

#include <string>

enum class TokenType {
    KEYWORD,
    SERIAL_KEYWORD,
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING,
    CHARACTER,
    BOOLEAN,
    EQUAL,
    EQUALITY,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    NOT_EQUAL,
    PLUS_EQUAL,
    MINUS_EQUAL,
    INCREMENT,
    DECREMENT,
    BANG,
    MINUS,
    PLUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
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
    std::string source;

    Token()
        : type(TokenType::EndOfFile), value(""), line(0), column(0), source("") {}

    Token(TokenType type, const std::string& value, int line = 0, int column = 0, const std::string& source = "")
        : type(type), value(value), line(line), column(column), source(source) {}
};