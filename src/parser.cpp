// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#include "parser.hpp"
#include "lexer.hpp"
#include "error_handler.hpp"

#include <cctype>
#include <iostream>
#include <algorithm>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens, const std::string& source)
    : tokens(tokens), source(source), position(0) {

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
        current = Token(TokenType::EndOfFile, "", current.line);
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
        ErrorHandler::report(errorMessage, current.value, current.line, current.column, this->source);
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
    auto left = parseLogicalNot();

    while (current.type == TokenType::KEYWORD && (current.value == "and" || current.value == "&&")) {
        Token op = current;

        advance();
        auto right = parseEquality();

        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalNot() {
    if (current.type == TokenType::KEYWORD && (current.value == "not" || current.value == "!")) {
        Token op = current;
        
        advance();

        auto right = parseLogicalOr();
        return std::make_unique<UnaryOpNode>(op, std::move(right));
    }

    return parseEquality();
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

    while (current.type == TokenType::GREATER || current.type == TokenType::GREATER_EQUAL || current.type == TokenType::LESS || current.type == TokenType::LESS_EQUAL) {
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

        if (current.type == TokenType::DOT) {
            advance();
            std::string methodName = current.value;
            
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

                consume(TokenType::RPAREN, "Expected ')' after method arguments");

                std::unique_ptr<ASTNode> methodCall;

                if (name == "serial") {
                    methodCall = std::make_unique<SerialFunctionsCallNode>(methodName, std::move(args), current.line, current.column, this->source);
                } else {
                    methodCall = std::make_unique<FunctionCallNode>(methodName, std::move(args), current.line, current.column, this->source);
                }

                return std::make_unique<MethodCallNode>(name, std::move(methodCall), current.line, current.column, this->source);
            } else {
                auto methodCall = std::make_unique<FunctionCallNode>(methodName, std::vector<std::unique_ptr<ExpressionNode>>(), current.line, current.column, this->source);

                return std::make_unique<MethodCallNode>(name, std::move(methodCall), current.line, current.column, this->source);
            }
        }

        if (current.type == TokenType::LBRACKET) {
            advance();

            auto indexExpression = parseExpression();

            consume(TokenType::RBRACKET, "Expected ']' after array index");

            if (!arraysNamesList.count(name)) {
                ErrorHandler::report("Undefined array name", name, current.line, current.column, this->source);
            }

            return std::make_unique<ArrayIndexNode>(name, std::move(indexExpression));
        }

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

            if (keywordsList.count(name)) {
                if (name == "int" || name == "str" || name == "float") {
                    return std::make_unique<TypeConversionCallNode>(name, std::move(args), current.line, current.column, this->source);
                } else {
                    return std::make_unique<BuiltInFunctionCallNode>(name, std::move(args), current.line, current.column, this->source);
                }
            } else {
                return std::make_unique<FunctionCallNode>(name, std::move(args), current.line, current.column, this->source);
            }
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

    ErrorHandler::report("Unexpected token in expression:", current.value, current.line, current.column, this->source);
    
    return nullptr;
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

            if (current.type == TokenType::LBRACKET) {
                body.push_back(parseArrayAssignment(name));
            } else if (current.type == TokenType::EQUAL) {
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

                body.push_back(std::make_unique<MethodCallNode>(name, std::move(methodCall), current.line, current.column, this->source));
            }
        } else if (current.type == TokenType::KEYWORD) {
            std::string keyword = current.value;

            if (keyword == "if") {
                body.push_back(parseIfStatement());
            } else if (keyword == "return") {
                body.push_back(parseReturnStatement());
            } else if (keyword == "every") {
                body.push_back(parseEveryStatement());
            } else if (keyword == "while") {
                body.push_back(parseWhileStatement());
            } else if (keyword == "for") {
                body.push_back(parseForStatement());
            } else if (keyword == "repeat") {
                body.push_back(parseRepeatStatement());
            } else if (keyword == "on_press") {
                body.push_back(parseOnPressStatement());
            } else if (keyword == "break" || keyword == "continue") {
                body.push_back(parseControlTransferStatements(keyword));
            } else if (keyword == "int" || keyword == "str" || keyword == "float") {
                body.push_back(parseTypeConversionCall(keyword));
            } else {
                advance();

                if (current.type == TokenType::DOT) {
                    advance();

                    std::string methodName = current.value;

                    advance();

                    std::unique_ptr<ASTNode> methodCall;

                    if (keyword == "serial") {
                        methodCall = parseSerialFunctionsCall(methodName);
                    } else {
                        methodCall = parseFunctionCall(methodName);
                    }
                    
                    body.push_back(std::make_unique<MethodCallNode>(keyword, std::move(methodCall), current.line, current.column, this->source));
                } else if (current.type == TokenType::LPAREN) {
                    if (keywordsList.count(keyword)) {
                        body.push_back(parseBuiltInFunctionCall(keyword));
                    } else {
                        body.push_back(parseFunctionCall(keyword));
                    }
                }
            }
        } else {
            advance();
        }
    }

    return body;
}

std::unique_ptr<ASTNode> Parser::parseUseStatement() {
    advance();

    std::string libName = current.value;

    return std::make_unique<UseNode>(libName);
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall(const std::string& funcName) {
    consume(TokenType::LPAREN, "Expected '(' after " + funcName);

    std::vector<std::unique_ptr<ExpressionNode>> args;
    
    while (current.type != TokenType::RPAREN && current.type != TokenType::EndOfFile) {
        if (current.type == TokenType::COMMA) {
            advance();
            
            if (current.type == TokenType::RPAREN) {
                ErrorHandler::report("Trailing comma found in function call arguments", current.value, current.line, current.column, this->source);
            }
        }

        if (current.type == TokenType::NUMBER || current.type == TokenType::SYMBOL || current.type == TokenType::KEYWORD || current.type == TokenType::STRING || current.type == TokenType::LPAREN) {
            args.push_back(parseExpression());
        } else {
            ErrorHandler::report("Unexpected token inside function call:", current.value, current.line, current.column, this->source);

            advance();

            continue;
        }
    }
    
    consume(TokenType::RPAREN, "Expected ')' after arguments in " + funcName);
    
    return std::make_unique<FunctionCallNode>(funcName, std::move(args), current.line, current.column, this->source);
}

std::unique_ptr<ASTNode> Parser::parseBuiltInFunctionCall(const std::string& funcName) {
    consume(TokenType::LPAREN, "Expected '(' after " + funcName);
    
    std::vector<std::unique_ptr<ExpressionNode>> args;
    
    while (current.type != TokenType::RPAREN && current.type != TokenType::EndOfFile) {
        if (current.type == TokenType::COMMA) {
            advance();
            
            if (current.type == TokenType::RPAREN) {
                ErrorHandler::report("Trailing comma found in function call arguments", current.value, current.line, current.column, this->source);
            }
        }

        if (current.type == TokenType::NUMBER || current.type == TokenType::SYMBOL || current.type == TokenType::KEYWORD || current.type == TokenType::STRING || current.type == TokenType::LPAREN) {
            args.push_back(parseExpression());
        } else {
            ErrorHandler::report("Unexpected token inside function call:", current.value, current.line, current.column, this->source);

            advance();
            continue;
        }
    }
    
    consume(TokenType::RPAREN, "Expected ')' after arguments in " + funcName);
    
    return std::make_unique<BuiltInFunctionCallNode>(funcName, std::move(args), current.line, current.column, this->source);
}

std::unique_ptr<ASTNode> Parser::parseSerialFunctionsCall(const std::string& funcName) {
    consume(TokenType::LPAREN, "Expected '(' after " + funcName);
    
    std::vector<std::unique_ptr<ExpressionNode>> args;
    
    while (current.type != TokenType::RPAREN && current.type != TokenType::EndOfFile) {
        if (current.type == TokenType::COMMA) {
            advance();
            
            if (current.type == TokenType::RPAREN) {
                ErrorHandler::report("Trailing comma found in function call arguments", current.value, current.line, current.column, this->source);
            }
        }

        if (current.type == TokenType::NUMBER || current.type == TokenType::SYMBOL || current.type == TokenType::KEYWORD || current.type == TokenType::STRING || current.type == TokenType::LPAREN) {
            args.push_back(parseExpression());
        } else {
            ErrorHandler::report("Unexpected token inside function call:", current.value, current.line, current.column, this->source);

            advance();
            continue;
        }
    }
    
    consume(TokenType::RPAREN, "Expected ')' after arguments in " + funcName);
    
    return std::make_unique<SerialFunctionsCallNode>(funcName, std::move(args), current.line, current.column, this->source);
}

std::unique_ptr<ASTNode> Parser::parseAssignment(const std::string& varName) {
    advance();

    if (current.type == TokenType::LBRACKET) {
        if (!arraysNamesList.count(varName)) {
            arraysNamesList.insert(varName);
        }

        auto listNode = parseArrayLiteral(varName);

        if (parsedVariables.count(varName)) {
            reassignedVariables.insert(varName);
        } else {
            parsedVariables.insert(varName);
        }

        if (current.type == TokenType::NEWLINE) {
            advance();
        }

        return listNode;
    }

    std::string raw = current.value;
    TokenType type_of_value = current.type;

    bool isConstantVar = false;
    bool isAllUpercase = std::all_of(varName.begin(), varName.end(), [] (unsigned char c) {
        return std::isupper(c) || c == '_';
    });

    auto expr_value = parseExpression();

    if (parsedVariables.count(varName)) {
        reassignedVariables.insert(varName);
    } else {
        parsedVariables.insert(varName);
    }

    if (isAllUpercase) {
        isConstantVar = true;
    }

    if (current.type == TokenType::NEWLINE) {
        advance();
    }

    return std::make_unique<VarAssignNode>(varName, std::move(expr_value), raw, type_of_value, isConstantVar, this->current.line, this->current.column, this->source);
}

std::unique_ptr<ASTNode> Parser::parseArrayLiteral(const std::string& name) {
    advance();

    std::vector<std::string> members;

    while (current.type != TokenType::RBRACKET) {
        auto element = parseExpression();
        std::string elementCpp = element->toCpp();

        while (!elementCpp.empty() && (elementCpp.back() == ';' || elementCpp.back() == '\n')) {
            elementCpp.pop_back();
        }

        members.push_back(elementCpp);

        if (current.type == TokenType::COMMA) {
            advance();
        } else {
            break;
        }
    }

    consume(TokenType::RBRACKET, "Expected ']' to close list literal");

    return std::make_unique<ArrayNode>(name, members);
}

std::unique_ptr<ASTNode> Parser::parseArrayAssignment(const std::string& name) {
    advance();

    if (!arraysNamesList.count(name)) {
        ErrorHandler::report("Undefined array name", name, current.line, current.column, this->source);
    }

    auto indexExpression = parseExpression();

    consume(TokenType::RBRACKET, "Expected ']' after array index");
    consume(TokenType::EQUAL, "Expected '=' after array index for assignment");

    auto valueExpression = parseExpression();

    if (current.type == TokenType::NEWLINE) {
        advance();
    }

    return std::make_unique<ArrayAssignNode>(name, std::move(indexExpression), std::move(valueExpression));
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

    return std::make_unique<FunctionCallNode>(keyword, std::move(args), current.line, current.column, this->source);
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

std::unique_ptr<ASTNode> Parser::parseControlTransferStatements(const std::string& statement) {
    advance();

    return std::make_unique<ControlTransferStatementsNode>(statement);
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

    consume(TokenType::INDENT, "Expected indentation after function");

    auto body = parseBlock();

    consume(TokenType::DEDENT, "Expected dedent at end of function body");

    currentParsingUserFunc = "";

    currentFuncParamNames.clear();
    funcParams.clear();

    if (funcName == "loop" || funcName == "start") {
        return std::make_unique<FunctionNode>(funcName, std::move(body), current.line, current.column, this->source);
    } else {
        userDefinedFunctionNames.insert(funcName);

        return std::make_unique<UserFuncNode>(funcName, std::move(params), std::move(body), current.line, current.column, this->source);
    }
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    advance();

    auto value = parseExpression();

    return std::make_unique<ReturnNode>(std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseEveryStatement() {
    advance();

    if (current.type != TokenType::NUMBER && current.type != TokenType::SYMBOL) {
        ErrorHandler::report("Expected a number or identifier for interval", current.value, current.line, current.column, this->source);
    }

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

std::unique_ptr<ASTNode> Parser::parseForStatement() {
    advance();

    if (current.type == TokenType::SYMBOL) {
        std::string varName = current.value;

        advance();

        if (current.value != "in") {
            ErrorHandler::report("Expected 'in' keyword", current.value, current.line, current.column, this->source);
        }

        advance();

        if (current.value != "range") {
            ErrorHandler::report("Expected 'range' after 'in'", current.value, current.line, current.column, this->source);
        }

        advance();
        consume(TokenType::LPAREN, "Expected '(' after 'range'");

        auto startExpression = parseExpression();
        std::unique_ptr<ExpressionNode> stopExpression;
        std::unique_ptr<ExpressionNode> stepExpression;

        if (current.type == TokenType::COMMA) {
            advance();
            stopExpression = parseExpression();
        } else {
            stopExpression = std::move(startExpression);
            startExpression = std::make_unique<LiteralNode>(Token(TokenType::NUMBER, "0", current.line));
        }

        if (current.type == TokenType::COMMA) {
            advance();
            stepExpression = parseExpression();
        } else {
            stepExpression = std::make_unique<LiteralNode>(Token(TokenType::NUMBER, "1", current.line));
        }

        consume(TokenType::RPAREN, "Expected ')' after range arguments");
        consume(TokenType::COLON, "Expected ':' after for-range");

        match(TokenType::NEWLINE);
        consume(TokenType::INDENT, "Expected indentation after for");

        auto body = parseBlock();
        consume(TokenType::DEDENT, "Expected dedent at end of for block");

        return std::make_unique<ForRangeNode>(varName, std::move(startExpression), std::move(stopExpression), std::move(stepExpression), std::move(body));
    }

    auto condition = parseExpression();

    consume(TokenType::COLON, "Expected ':' after while condition");
    match(TokenType::NEWLINE);

    consume(TokenType::INDENT, "Expected indentation after while");

    auto body = parseBlock();
    consume(TokenType::DEDENT, "Expected dedent at end of while block");

    return std::make_unique<ForNode>(std::move(condition), std::move(body));
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

std::unique_ptr<ASTNode> Parser::parseTypeConversionCall(const std::string& funcName) {
    consume(TokenType::LPAREN, "Expected '(' after " + funcName);

    std::vector<std::unique_ptr<ExpressionNode>> args;
    
    while (current.type != TokenType::RPAREN && current.type != TokenType::EndOfFile) {
        if (current.type == TokenType::COMMA) {
            advance();
            
            if (current.type == TokenType::RPAREN) {
                ErrorHandler::report("Trailing comma found in function call arguments", current.value, current.line, current.column, this->source);
            }
        }

        if (current.type == TokenType::NUMBER || current.type == TokenType::SYMBOL || current.type == TokenType::KEYWORD || current.type == TokenType::STRING || current.type == TokenType::LPAREN) {
            args.push_back(parseExpression());
        } else {
            ErrorHandler::report("Unexpected token inside function call:", current.value, current.line, current.column, this->source);

            advance();

            continue;
        }
    }
    
    consume(TokenType::RPAREN, "Expected ')' after arguments in " + funcName);

    return std::make_unique<TypeConversionCallNode>(funcName, std::move(args), current.line, current.column, this->source);
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

            if (keyword == "use") {
                program.push_back(parseUseStatement());
            } else if (keyword == "if") {
                program.push_back(parseIfStatement());
            } else if (keyword == "fn") {
                program.push_back(parseUserFuncDefinition());
            } else if (keyword == "every") {
                program.push_back(parseEveryStatement());
            } else if (keyword == "while") {
                program.push_back(parseWhileStatement());
            } else if (keyword == "for") {
                program.push_back(parseForStatement());
            } else if (keyword == "repeat") {
                program.push_back(parseRepeatStatement());
            } else if (keyword == "on_press") {
                program.push_back(parseOnPressStatement());
            } else if (keyword == "break" || keyword == "continue") {
                program.push_back(parseControlTransferStatements(keyword));
            } else if (keyword == "int" || keyword == "str" || keyword == "float") {
                program.push_back(parseTypeConversionCall(keyword));
            } else if (keyword == "pass") {
                advance();
            } else if (keywordsList.count(keyword)) {
                program.push_back(parseKeywordFunctionCall(keyword));
            }
        }

        if (position == start_position) {
            std::string badToken = current.value;

            advance();
            ErrorHandler::report("Unexpected token in expression", badToken, current.line, current.column, this->source);
        }
    }

    return program;
}