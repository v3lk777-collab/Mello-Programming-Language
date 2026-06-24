// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
    std::string source;
    size_t position;
    size_t length;
    char current;
    
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