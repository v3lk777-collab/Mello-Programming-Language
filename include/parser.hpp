// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once
#include "ast.hpp"
#include "token.hpp"
#include <vector>
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    size_t position;
    Token current;

    void advance();
    bool match(TokenType type);
    void consume(TokenType type, const std::string& errorMessage);

    std::vector<std::unique_ptr<ASTNode>> parseBlock();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseLogicalOr();
    std::unique_ptr<ExpressionNode> parseLogicalAnd();
    std::unique_ptr<ExpressionNode> parseEquality();
    std::unique_ptr<ExpressionNode> parseComparison();
    std::unique_ptr<ExpressionNode> parseTerm();
    std::unique_ptr<ExpressionNode> parseFactor();
    std::unique_ptr<ExpressionNode> parsePrimary();
    std::unique_ptr<ASTNode> parseFunctionCall(const std::string& func_name);
    std::unique_ptr<ASTNode> parseAssignment(const std::string& var_name);
    std::unique_ptr<ASTNode> parseFunctionDefinition(const std::string& keyword);
    std::unique_ptr<ASTNode> parseKeywordFunctionCall(const std::string& keyword);
    std::unique_ptr<ASTNode> parseIfStatement();
    std::vector<std::unique_ptr<ASTNode>> parseElseChain();
    std::unique_ptr<ASTNode> parseUserFuncDefinition();
    std::unique_ptr<ASTNode> parseReturnStatement();
    std::unique_ptr<ASTNode> parseEveryStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::unique_ptr<ASTNode> parseRepeatStatement();
    std::unique_ptr<ASTNode> parseOnPressStatement();

public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<ASTNode>> parse();
};