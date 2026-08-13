// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#include "lexer.hpp"

#include <iostream>

Lexer::Lexer(std::string source) {
    this->source = std::move(source);
    this->position = 0;

    this->length = this->source.length();
    this->current = (this->length > 0) ? this->source[0] : '\0';
    this->currentLine = 1;
    this->currentColumn = 0;

    this->indent_stack.push_back(0);
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
    currentColumn++;

    if (current == '\n') {
        currentLine++;
        currentColumn = 0;
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
            int current_indent = 0;

            while (current == ' ' || current == '\t') {
                if (current == '\t') {
                    current_indent += 4;
                } else {
                    current_indent += 1;
                }

                advance();
            }

            if (current == '\n' || current == '#') {
                skipComment();

                if (current == '\n') {
                    advance();
                }
                
                continue;
            }

            int last_indent = indent_stack.back();

            if (current_indent > last_indent) {
                indent_stack.push_back(current_indent);
                tokens.push_back({TokenType::INDENT, std::to_string(current_indent), currentLine, currentColumn});
            } else if (current_indent < last_indent) {
                while (!indent_stack.empty() && indent_stack.back() > current_indent) {
                    indent_stack.pop_back();
                    tokens.push_back({TokenType::DEDENT, "", currentLine, currentColumn});
                }
                
                if (indent_stack.empty() || indent_stack.back() != current_indent) {
                    std::cerr << "Indentation Error: Unindent does not match any outer level\n";
                    exit(1);
                }
            }

            isStartOfLine = false;
        }

        skipWhitespace();
        skipComment();

        if (fileIsEmpty()) {
            break;
        }

        if (isdigit(current) || (current == '.' && isdigit(peek()))) {
            std::string number;
            bool hasDot = false;

            if (current == '0' && (peek() == 'x' || peek() == 'X')) {
                number += current;

                advance();

                number += current;

                advance();

                while (std::isxdigit(current)) {
                    number += current;
                    advance();
                }
            } else {
                while (isdigit(current) || (current == '.' && !hasDot)) {
                    if (current == '.') {
                        hasDot = true;
                    }
                    
                    number += current;
                    advance();
                }


                if (current == 's' || current == 'm' || current == 'h') {
                    number += current;
                    advance();
                }
            }

            tokens.push_back({TokenType::NUMBER, number, currentLine, currentColumn});
        } else if (current == '\n') {
            tokens.push_back({TokenType::NEWLINE, "\n", currentLine, currentColumn});
            advance();
            
            isStartOfLine = true;
        } else if (current == '"') {
            std::string str;
            advance();

            while (current != '"' && current != '\0') {
                str += current;
                advance();
            }

            advance();
            tokens.push_back({TokenType::STRING, str, currentLine, currentColumn});
        } else if (current == '.') {
            tokens.push_back({TokenType::DOT, ".", currentLine, currentColumn});
            advance();
        } else if (current == ')') {
            std::string paren(1, current);
            tokens.push_back({TokenType::RPAREN, paren, currentLine, currentColumn});
            advance();
        } else if (current == '(') {
            std::string paren(1, current);
            tokens.push_back({TokenType::LPAREN, paren, currentLine, currentColumn});
            advance();
        } else if (current == ']') {
            std::string paren(1, current);
            tokens.push_back({TokenType::RBRACKET, paren, currentLine, currentColumn});
            advance();
        } else if (current == '[') {
            std::string paren(1, current);
            tokens.push_back({TokenType::LBRACKET, paren, currentLine, currentColumn});
            advance();
        } else if (current == '=') {
            if (peek() == '=') {
                advance();
                advance();
                tokens.push_back({TokenType::EQUALITY, "==", currentLine, currentColumn});
            } else {
                tokens.push_back({TokenType::EQUAL, "=", currentLine, currentColumn});
                advance();
            }
        } else if (current == '+') {
            if (peek() == '+') {
                advance();
                advance();

                tokens.push_back({TokenType::PLUS_PLUS, "++", currentLine, currentColumn});
            } else if (peek() == '=') {
                advance();
                advance();

                tokens.push_back({TokenType::PLUS_EQUAL, "+=", currentLine, currentColumn});
            } else {
                tokens.push_back({TokenType::PLUS, "+", currentLine, currentColumn});
                advance();
            }
        } else if (current == '-') {
            if (peek() == '-') {
                advance();
                advance();

                tokens.push_back({TokenType::MINUS_MINUS, "--", currentLine, currentColumn});
            } else if (peek() == '=') {
                advance();
                advance();
                
                tokens.push_back({TokenType::MINUS_EQUAL, "-=", currentLine, currentColumn});
            } else {
                tokens.push_back({TokenType::MINUS, "-", currentLine, currentColumn});
                advance();
            }
        } else if (current == '*') {
            std::string symbol(1, current);
            tokens.push_back({TokenType::MUL, symbol, currentLine, currentColumn});
            advance();
        } else if (current == '/') {
            std::string symbol(1, current);
            tokens.push_back({TokenType::DIV, symbol, currentLine, currentColumn});
            advance();
        } else if (current == ':') {
            std::string symbol(1, current);
            tokens.push_back({TokenType::COLON, symbol, currentLine, currentColumn});
            advance();
        } else if (current == ',') {
            std::string symbol(1, current);
            tokens.push_back({TokenType::COMMA, symbol, currentLine, currentColumn});
            advance();
        } else if (current == '>') {
            if (peek() == '=') {
                advance();
                advance();
                tokens.push_back({TokenType::GREATER_EQUAL, ">=", currentLine, currentColumn});
            } else {
                std::string symbol(1, current);
                tokens.push_back({TokenType::GREATER, ">", currentLine, currentColumn});
                advance();
            }
        } else if (current == '<') {
            if (peek() == '=') {
                advance();
                advance();
                tokens.push_back({TokenType::LESS_EQUAL, "<=", currentLine, currentColumn});
            } else {
                std::string symbol(1, current);
                tokens.push_back({TokenType::LESS, "<", currentLine, currentColumn});
                advance();
            }
        } else if (current == '!') {
            if (peek() == '=') {
                advance();
                advance();
                tokens.push_back({TokenType::NOT_EQUAL, "!=", currentLine, currentColumn});
            } else {
                tokens.push_back({TokenType::BANG, "!", currentLine, currentColumn});
                advance();
            }
        } else if (current == '&') {
            if (peek() == '&') {
                advance();
                advance();
                tokens.push_back({TokenType::KEYWORD, "&&", currentLine, currentColumn});
            } else {
                advance();
            }
        } else if (current == '|') {
            if (peek() == '|') {
                advance();
                advance();
                tokens.push_back({TokenType::KEYWORD, "||", currentLine, currentColumn});
            } else {
                advance();
            }
        } else if (isalpha(current) || current == '_') {
            std::string identifier;

            while (isalnum(current) || current == '_') {
                identifier += current;
                advance();
            }

            if (keywordsList.find(identifier) != keywordsList.end()) {
                tokens.push_back({TokenType::KEYWORD, identifier, currentLine, currentColumn});
            }  else {
                tokens.push_back({TokenType::SYMBOL, identifier, currentLine, currentColumn});
            }
        } else {
            advance();
        }
    }

    while (indent_stack.size() > 1) {
        indent_stack.pop_back();
        tokens.push_back({TokenType::DEDENT, "", currentLine, currentColumn});
    }

    tokens.push_back({TokenType::EndOfFile, "", currentLine, currentColumn});

    return tokens;
}