// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#include "parser.hpp"
#include "lexer.hpp"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {
    if (!tokens.empty()) {
        current = tokens[0];
    }
}

void Parser::advance() {
    if (position < tokens.size()) {
        position++;
    }
    
    if (position < tokens.size()) {
        current = tokens[position];
    } else {
        current = Token(TokenType::EndOfFile, "");
    }
}

bool Parser::match(TokenType type) {
    if (current.type == type) {
        advance();

        return true;
    }
    
    return false;
}

void Parser::consume(TokenType type, const std::string& errorMessage) {
    if (current.type == type) {
        advance();
    } else {
        throw std::runtime_error("Syntax Error at token '" + current.value + "': " + errorMessage);
    }
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();

    while (current.type == TokenType::KEYWORD && (current.value == "or" || current.value == "||")) {
        Token op = current;
        advance();
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalAnd() {
    auto left = parseEquality();

    while (current.type == TokenType::KEYWORD && (current.value == "and" || current.value == "&&")) {
        Token op = current;
        advance();
        auto right = parseEquality();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseEquality() {
    auto left = parseComparison();

    while (current.type == TokenType::EQUALITY || current.type == TokenType::NOT_EQUAL) {
        Token op = current;
        advance();
        auto right = parseComparison();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseComparison() {
    auto left = parseTerm();

    while (current.type == TokenType::GREATER || current.type == TokenType::GREATER_EQUAL ||
           current.type == TokenType::LESS || current.type == TokenType::LESS_EQUAL) {
        Token op = current;
        advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (current.type == TokenType::PLUS || current.type == TokenType::MINUS) {
        Token op = current;
        advance();
        auto right = parseFactor();

        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseFactor() {
    auto left = parsePrimary();

    while (current.type == TokenType::MUL || current.type == TokenType::DIV) {
        Token op = current;
        advance();
        auto right = parsePrimary();

        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parsePrimary() {
    if (current.type == TokenType::NUMBER) {
        Token t = current;
        advance();
        return std::make_unique<LiteralNode>(t);
    }

    if (current.type == TokenType::SYMBOL || current.type == TokenType::KEYWORD) {
        Token t = current;
        std::string name = current.value;
        advance();

        if (current.type == TokenType::LPAREN) {
            advance();

            std::vector<std::unique_ptr<ExpressionNode>> args;
            if (current.type != TokenType::RPAREN) {
                args.push_back(parseExpression());
                while (current.type == TokenType::COMMA) {
                    advance();
                    args.push_back(parseExpression());
                }
            }

            consume(TokenType::RPAREN, "Expected ')' after function arguments");

            return std::make_unique<FunctionCallNode>(name, std::move(args));
        }

        return std::make_unique<LiteralNode>(t);
    }
    
    if (current.type == TokenType::STRING) {
        Token t = current;
        t.value = "\"" + t.value + "\"";

        advance();

        return std::make_unique<LiteralNode>(t);
    }

    if (current.type == TokenType::LPAREN) {
        advance();

        auto expr = parseExpression();

        consume(TokenType::RPAREN, "Expected ')' in expression");

        return std::make_unique<GroupNode>(std::move(expr));
    }

    throw std::runtime_error("Unexpected token in expression: " + current.value);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseBlock() {
    std::vector<std::unique_ptr<ASTNode>> body;

    while (current.type != TokenType::DEDENT && current.type != TokenType::EndOfFile) {
        if (match(TokenType::NEWLINE)) {
            continue;
        }

        if (current.type == TokenType::SYMBOL) {
            std::string name = current.value;
            advance();

            if (current.type == TokenType::EQUAL) {
                body.push_back(parseAssignment(name));
            } else if (current.type == TokenType::LPAREN) {
                body.push_back(parseFunctionCall(name));
            } else if (current.type == TokenType::PLUS_EQUAL) {
                advance();
                std::string val = current.value;
                advance();
                reassignedVariables.insert(name);
                body.push_back(std::make_unique<CompoundAssignNode>(name, "+=", val));
            } else if (current.type == TokenType::MINUS_EQUAL) {
                advance();
                std::string val = current.value;
                advance();
                reassignedVariables.insert(name);
                body.push_back(std::make_unique<CompoundAssignNode>(name, "-=", val));
            } else if (current.type == TokenType::PLUS_PLUS) {
                advance();
                reassignedVariables.insert(name);
                body.push_back(std::make_unique<CompoundAssignNode>(name, "++", ""));
            } else if (current.type == TokenType::MINUS_MINUS) {
                advance();
                reassignedVariables.insert(name);
                body.push_back(std::make_unique<CompoundAssignNode>(name, "--", ""));
            } else if (current.type == TokenType::DOT) {
                advance();
                
                std::string methodName = current.value;
                advance();
                
                auto methodCall = parseFunctionCall(methodName);
                
                body.push_back(std::make_unique<MethodCallNode>(name, std::move(methodCall)));
            }
        }

        else if (current.type == TokenType::KEYWORD) {
            std::string inner_keyword = current.value;
            if (inner_keyword == "if") {
                body.push_back(parseIfStatement());
            } else if (inner_keyword == "return") {
                body.push_back(parseReturnStatement());
            } else if (inner_keyword == "every") {
                body.push_back(parseEveryStatement());
            } else if (inner_keyword == "while") {
                body.push_back(parseWhileStatement());
            } else if (inner_keyword == "repeat") {
                body.push_back(parseRepeatStatement());
            } else if (inner_keyword == "on_press") {
                body.push_back(parseOnPressStatement());
            } else if (keywordsList.count(inner_keyword)) {
                advance();
                body.push_back(parseFunctionCall(inner_keyword));
            } else {
                advance();
            }
        } else {
            advance();
        }
    }
    return body;
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall(const std::string& func_name) {
    consume(TokenType::LPAREN, "Expected '(' after " + func_name);
    
    std::vector<std::unique_ptr<ExpressionNode>> args;
    
    while (current.type != TokenType::RPAREN && current.type != TokenType::EndOfFile) {
        if (current.type == TokenType::COMMA) {
            advance();
            continue;
        }

        if (current.type == TokenType::NUMBER || current.type == TokenType::SYMBOL || current.type == TokenType::KEYWORD || current.type == TokenType::STRING) {
            Token token = current;

            if (current.type == TokenType::STRING) {
                token.value = "\"" + current.value + "\"";
                args.push_back(std::make_unique<LiteralNode>(token));
            } else {
                args.push_back(std::make_unique<LiteralNode>(token));
            }
            advance();
        } else {
            throw std::runtime_error("Unexpected token inside function call: " + current.value);
        }
    }
    
    consume(TokenType::RPAREN, "Expected ')' after arguments in " + func_name);
    
    return std::make_unique<FunctionCallNode>(func_name, std::move(args));
}

std::unique_ptr<ASTNode> Parser::parseAssignment(const std::string& var_name) {
    advance(); 

    std::string raw = current.value;
    TokenType type_of_value = current.type;

    auto expr_value = parseExpression(); 

    if (parsedVariables.count(var_name)) {
        reassignedVariables.insert(var_name);
    } else {
        parsedVariables.insert(var_name);
    }

    if (current.type == TokenType::NEWLINE) {
        advance();
    }

    return std::make_unique<VarAssignNode>(var_name, std::move(expr_value), raw, type_of_value);
}

std::unique_ptr<ASTNode> Parser::parseFunctionDefinition(const std::string& keyword) {
    advance();

    consume(TokenType::COLON, "Expected ':' to start function body");

    if (current.type == TokenType::NEWLINE && current.type != TokenType::EndOfFile && current.type != TokenType::DEDENT) {
        advance();
    }

    consume(TokenType::INDENT, "Expected indentation after function definition");

    auto body = parseBlock();

    consume(TokenType::DEDENT, "Expected dedent at the end of function body");

    return std::make_unique<FunctionNode>(keyword, std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseKeywordFunctionCall(const std::string& keyword) {
    advance();
    consume(TokenType::LPAREN, "Expected '(' after '" + keyword + "'");

    std::vector<std::unique_ptr<ExpressionNode>> args;
    
    while (current.type == TokenType::NUMBER || current.type == TokenType::SYMBOL || current.type == TokenType::STRING || current.type == TokenType::KEYWORD) {
        args.push_back(std::make_unique<LiteralNode>(current));
        advance();

        if (current.type == TokenType::COMMA) {
            advance();
        }
    }

    consume(TokenType::RPAREN, "Expected ')' after arguments in '" + keyword + "'");

    return std::make_unique<FunctionCallNode>(keyword, std::move(args));
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    advance();
    
    auto condition = parseExpression();

    consume(TokenType::COLON, "Expected ':' after if condition");

    match(TokenType::NEWLINE);
    consume(TokenType::INDENT, "Expected indentation after 'if'");

    auto thenBody = parseBlock();

    consume(TokenType::DEDENT, "Expected dedent after 'if' body");

    while (current.type == TokenType::NEWLINE) {
        advance();
    }

    auto ifNode = std::make_unique<IfNode>(std::move(condition), std::move(thenBody));

    ifNode->setElseBody(parseElseChain());

    return ifNode;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseElseChain() {
    std::vector<std::unique_ptr<ASTNode>> result;

    if (current.type != TokenType::KEYWORD) {
        return result;
    }

    if (current.value == "elif") {
        advance();
        
        auto condition = parseExpression();

        consume(TokenType::COLON, "Expected ':' after elif condition");

        match(TokenType::NEWLINE);
        consume(TokenType::INDENT, "Expected indentation after elif");

        auto body = parseBlock();

        consume(TokenType::DEDENT, "Expected dedent after elif body");

        while (current.type == TokenType::NEWLINE) {
            advance();
        }

        auto elifNode = std::make_unique<IfNode>(std::move(condition), std::move(body));

        elifNode->setElseBody(parseElseChain());

        result.push_back(std::move(elifNode));

    } else if (current.value == "else") {
        advance();

        consume(TokenType::COLON, "Expected ':' after else");

        match(TokenType::NEWLINE);
        consume(TokenType::INDENT, "Expected indentation after else");

        auto body = parseBlock();

        consume(TokenType::DEDENT, "Expected dedent after else body");
        
        for (auto& node : body) {
            result.push_back(std::move(node));
        }
    }

    return result;
}

std::unique_ptr<ASTNode> Parser::parseUserFuncDefinition() {
    advance();
    std::string funcName = current.value;

    advance();
    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<std::string> params;
    while (current.type != TokenType::RPAREN && current.type != TokenType::EndOfFile) {
        if (current.type == TokenType::COMMA) {
            advance();
            continue;
        }

        params.push_back(current.value);
        advance();
    }

    consume(TokenType::RPAREN, "Expected ')' after parameters");
    consume(TokenType::COLON, "Expected ':' after function signature");
    match(TokenType::NEWLINE);

    currentParsingUserFunc = funcName;
    currentFuncParamNames = params;
    userFuncOutputParams[funcName] = std::vector<bool>(params.size(), false);
    userFuncInputParams[funcName] = std::vector<bool>(params.size(), false);

    for (const auto& p : params) {
        funcParams.insert(p);
    }

    consume(TokenType::INDENT, "Expected indentation after func");

    auto body = parseBlock();

    consume(TokenType::DEDENT, "Expected dedent at end of func body");

    currentParsingUserFunc = "";

    currentFuncParamNames.clear();
    funcParams.clear();

    return std::make_unique<UserFuncNode>(funcName, std::move(params), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    advance();
    std::string value = current.value;
    advance();

    return std::make_unique<ReturnNode>(value);
}

std::unique_ptr<ASTNode> Parser::parseEveryStatement() {
    advance();

    std::string interval = current.value; 
    advance(); 

    consume(TokenType::COLON, "Expected ':' after every interval");
    match(TokenType::NEWLINE);
    
    consume(TokenType::INDENT, "Expected indentation after every");

    auto body = parseBlock();

    consume(TokenType::DEDENT, "Expected dedent at end of every block");
    
    return std::make_unique<EveryNode>(interval, std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    advance();
    
    auto condition = parseExpression();

    consume(TokenType::COLON, "Expected ':' after while condition");
    match(TokenType::NEWLINE);
    consume(TokenType::INDENT, "Expected indentation after while");
    auto body = parseBlock();
    consume(TokenType::DEDENT, "Expected dedent at end of while block");
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseRepeatStatement() {
    advance();

    std::string count = current.value;
    advance();

    consume(TokenType::COLON, "Expected ':' after repeat count");
    match(TokenType::NEWLINE);
    
    consume(TokenType::INDENT, "Expected indentation after repeat");
    auto body = parseBlock();
    consume(TokenType::DEDENT, "Expected dedent at end of repeat block");
    
    return std::make_unique<RepeatNode>(count, std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseOnPressStatement() {
    advance();

    std::string pin = current.value;

    advance();

    consume(TokenType::COLON, "Expected ':' after 'on_press' signature");

    match(TokenType::NEWLINE);
    consume(TokenType::INDENT, "Expected indentation after 'on_press'");

    auto body = parseBlock();
    consume(TokenType::DEDENT, "Expected dedent at end of 'on_press' block");

    return std::make_unique<OnPressNode>(pin, std::move(body));
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> program;

    while (current.type != TokenType::EndOfFile) {
        size_t start_position = position;
        
        if (match(TokenType::NEWLINE)) {
            continue;
        }

        if (current.type == TokenType::SYMBOL) {
            std::string name = current.value;
            advance();
            if (current.type == TokenType::EQUAL) {
                program.push_back(parseAssignment(name));
            } else if (current.type == TokenType::LPAREN) {
                program.push_back(parseFunctionCall(name));
            }
        }

        if (current.type == TokenType::KEYWORD) {
            std::string keyword = current.value;

            if (keyword == "start" || keyword == "loop") {
                program.push_back(parseFunctionDefinition(keyword));
            }

            else if (keyword == "if") {
                program.push_back(parseIfStatement());
            }

            else if (keyword == "func") {
                program.push_back(parseUserFuncDefinition());
            }

            else if (keyword == "every") {
                program.push_back(parseEveryStatement());
            }

            else if (keyword == "while") {
                program.push_back(parseWhileStatement());
            }

            else if (keyword == "repeat") {
                program.push_back(parseRepeatStatement());
            }

            else if (keyword == "on_press") {
                program.push_back(parseOnPressStatement());
            }

            else if (keywordsList.count(keyword)) {
                program.push_back(parseKeywordFunctionCall(keyword));
            }
        }

        if (position == start_position) {
            std::string badToken = current.value;

            advance();
            throw std::runtime_error("Unexpected token: " + badToken);
        }
    }

    return program;
}