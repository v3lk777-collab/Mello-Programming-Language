/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
*/

#include "lexer.hpp"

#include "error_handler.hpp"

#include <iostream>

Lexer::Lexer(std::string source) {
    this->source = std::move(source);
    this->position = 0;

    this->length = this->source.length();
    this->current = (this->length > 0) ? this->source[0] : '\0';

    this->line = 1;
    this->column = 0;

    this->indentationStack.push_back(0);
    this->isStartOfLine = true;
}

char Lexer::peek() {
    if (position + 1 < length) {
        return source[position + 1];
    } else {
        return '\0';
    }
}

void Lexer::advance() {
    column++;

    if (current == '\n') {
        line++;
        column = 0;
    }
    
    do {
        position++;

        if (position < length) {
            current = source[position];
        } else {
            current = '\0';
        }
    } while (current == '\r');
}

void Lexer::skipWhitespace() {
    while (current == ' ' || current == '\t' || current == '\r') {
        advance();
    }
}

void Lexer::skipComment() {
    if (current == '#') {
        while (current != '\n' && current != '\0') {
            advance();
        }
    }
}

bool Lexer::fileIsEmpty() {
    if (current == '\0') {
        return true;
    }

    return false;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (current != '\0') {
        if (isStartOfLine) {
            int currentIndex = 0;

            while (current == ' ' || current == '\t') {
                if (current == '\t') {
                    currentIndex += 4;
                } else {
                    currentIndex += 1;
                }

                advance();
            }

            if (current == '\n' || current == '#') {
                skipComment();

                if (current == '\n') {
                    advance();
                }

                isStartOfLine = true;
                continue;
            }

            int lastIndex = indentationStack.back();

            if (currentIndex > lastIndex) {
                indentationStack.push_back(currentIndex);

                tokens.push_back({TokenType::INDENT, "", line, column, source});
            } else if (currentIndex < lastIndex) {
                while (!indentationStack.empty() && indentationStack.back() > currentIndex) {
                    indentationStack.pop_back();

                    tokens.push_back({TokenType::DEDENT, "", line, column, source});
                }

                if (indentationStack.empty() || indentationStack.back() != currentIndex) {
                    ErrorHandler::report("Indentation Error: Unindent does not match any outer level", "", line, column, source);
                }
            }

            isStartOfLine = false;
        }

        if (current == ' ' || current == '\t' || current == '\r') {
            skipWhitespace();
            continue;
        }

        if (current == '#') {
            skipComment();
            continue;
        }

        if (current == '\n') {
            tokens.push_back({TokenType::NEWLINE, "", line, column, source});

            advance();

            isStartOfLine = true;
            continue;
        }

        if (fileIsEmpty()) {
            break;
        }

        switch (current) {
        case '+':
            if (peek() == '+') {
                tokens.push_back({TokenType::INCREMENT, "++", line, column, source});

                advance();
            } else if (peek() == '=') {
                tokens.push_back({TokenType::PLUS_EQUAL, "+=", line, column, source});

                advance();
            } else {
                tokens.push_back({TokenType::PLUS, "+", line, column, source});
            }

            advance();
            continue;

        case '-':
            if (peek() == '-') {
                tokens.push_back({TokenType::DECREMENT, "--", line, column, source});

                advance();
            } else if (peek() == '=') {
                tokens.push_back({TokenType::MINUS_EQUAL, "-=", line, column, source});

                advance();
            } else {
                tokens.push_back({TokenType::MINUS, "-", line, column, source});
            }

            advance();
            continue;

        case '*':
            tokens.push_back({TokenType::MULTIPLY, "*", line, column, source});

            advance();
            continue;

        case '/':
            tokens.push_back({TokenType::DIVIDE, "/", line, column, source});

            advance();
            continue;

        case '%':
            tokens.push_back({TokenType::MODULO, "%", line, column, source});

            advance();
            continue;

        case '(':
            tokens.push_back({TokenType::LPAREN, "(", line, column, source});

            advance();
            continue;

        case ')':
            tokens.push_back({TokenType::RPAREN, ")", line, column, source});

            advance();
            continue;

        case '[':
            tokens.push_back({TokenType::LBRACKET, "[", line, column, source});

            advance();
            continue;

        case ']':
            tokens.push_back({TokenType::RBRACKET, "]", line, column, source});

            advance();
            continue;

        case ',':
            tokens.push_back({TokenType::COMMA, ",", line, column, source});

            advance();
            continue;

        case ':':
            tokens.push_back({TokenType::COLON, ":", line, column, source});

            advance();
            continue;

        case '.':
            tokens.push_back({TokenType::DOT, ".", line, column, source});

            advance();
            continue;
        }

        if (current == '=') {
            if (peek() == '=') {
                tokens.push_back({TokenType::EQUALITY, "==", line, column, source});

                advance();
            } else {
                tokens.push_back({TokenType::EQUAL, "=", line, column, source});
            }

            advance();
            continue;
        }

        if (current == '>') {
            if (peek() == '=') {
                tokens.push_back({TokenType::GREATER_EQUAL, ">=", line, column, source});

                advance();
            } else {
                tokens.push_back({TokenType::GREATER, ">", line, column, source});
            }

            advance();
            continue;
        }

        if (current == '<') {
            if (peek() == '=') {
                tokens.push_back({TokenType::LESS_EQUAL, "<=", line, column, source});

                advance();
            } else {
                tokens.push_back({TokenType::LESS, "<", line, column, source});
            }

            advance();
            continue;
        }

        if (isdigit(current)) {
            std::string number;
            bool isFloat = false;

            while ((current != '\0' && current != '\n') && (isdigit(current) || current == '.')) {
                if (current == '.') {
                    if (!isFloat) {
                        isFloat = true;
                    } else {
                        ErrorHandler::report("Lexical Error: You can't type two dots in the same number", "..", line, column, source);
                    }
                }

                number += current;

                advance();
            }

            if (isFloat) {
                tokens.push_back({TokenType::FLOAT, number, line, column, source});
            } else {
                tokens.push_back({TokenType::INTEGER, number, line, column, source});
            }

            continue;
        }

        if (current == '"' || current == '\'') {
            std::string word;
            char quoteType = current;

            advance();

            while (current != '\0' && current != '\n' && current != quoteType) {
                word += current;

                advance();
            }

            if (current == quoteType) {
                if (current == '\'' && word.length() == 1) {
                    tokens.push_back({TokenType::CHARACTER, word, line, column, source});
                } else {
                    tokens.push_back({TokenType::STRING, word, line, column, source});
                }

                advance();
            } else {
                ErrorHandler::report("Lexical Error: Unterminated string literal", "", line, column, source);
            }

            continue;
        }

        if (isalpha(current) || current == '_') {
            std::string word;

            while (current != '\0' && (isalnum(current) || current == '_')) {
                word += current;

                advance();
            }

            if (keywordsList.count(word) > 0) {
                tokens.push_back({TokenType::KEYWORD, word, line, column, source});
            } else {
                tokens.push_back({TokenType::IDENTIFIER, word, line, column, source});
            }

            continue;
        }

        ErrorHandler::report("Lexical Error: Unknown character", std::string(1, current), line, column, source);

        advance(); 
    }

    while (indentationStack.size() > 1) {
        indentationStack.pop_back();
        tokens.push_back({TokenType::DEDENT, "", line, column, source});
    }

    tokens.push_back({TokenType::EndOfFile, "", line, column, source});

    return tokens;
}