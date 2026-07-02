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
    INDENT,
    DEDENT,
    NEWLINE,
    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;

    Token()
        : type(TokenType::EndOfFile), value("") {}

    Token(TokenType type, const std::string& value)
        : type(type), value(value) {}
};