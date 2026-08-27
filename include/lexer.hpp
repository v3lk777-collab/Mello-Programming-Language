// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once

#include "token.hpp"

#include <set>
#include <string>
#include <vector>
#include <unordered_set>

inline const std::unordered_set<std::string> keywordsList = {
    "start", "loop", "use", "wait", "turn_on", "turn_off", "if", "elif", "else", "len",
    "write", "read", "serial", "scale", "fn", "return", "and", "or", "not", "range",
    "every", "while", "for", "in", "repeat", "on_press", "toggle", "break", "continue",
    "sleep", "pass", "int", "str", "float", "save_memory", "read_memory"
};

inline const std::unordered_set<std::string> serialKeywordsList = {
    "start", "print", "println", "read", "write", "available", "availableForWrite", "end",
    "find", "findUntil", "waitUntilSend", "parseFloat", "parseInt", "peek", "readStringUntil"
};

class Lexer {
private:
    char current;
    size_t length;
    size_t position;
    int currentLine;
    int currentColumn;
    std::string source;

    bool isStartOfLine;
    std::vector<int> indentStack;

private:
    char peek();
    void advance();
    void skipWhitespace();
    void skipComment();
    bool fileIsEmpty();

public:
    std::vector<Token> tokenize();
    Lexer(std::string source);
};