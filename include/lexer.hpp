// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once
#include "token.hpp"
#include <unordered_set>
#include <string>
#include <vector>
#include <set>

inline const std::unordered_set<std::string> keywordsList = {
    "start", "loop", "wait", "turn_on", "turn_off", "if", "elif", "else", "read_serial",
    "write", "read", "serial", "print", "println", "scale", "func", "return", "available_serial",
    "every", "while", "repeat", "on_press", "and", "or"
};

class Lexer {
private:
    char current;
    size_t length;
    size_t position;
    std::string source;
    
    bool isStartOfLine;
    std::vector<int> indent_stack;

    char peek();
    void advance();
    void skipWhitespace();
    void skipComment();
    bool fileIsEmpty();

public:
    std::vector<Token> tokenize();

    Lexer(const std::string& source);
};