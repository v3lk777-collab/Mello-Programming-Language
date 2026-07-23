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
    Token current;
    size_t position;
    std::string source;
    std::vector<Token> tokens;

private:
    void advance();
    bool match(TokenType type);
    void consume(TokenType type, const std::string& errorMessage);

private:
    std::vector<std::unique_ptr<ASTNode>> parseBlock();

private:
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseLogicalOr();
    std::unique_ptr<ExpressionNode> parseLogicalAnd();
    std::unique_ptr<ExpressionNode> parseLogicalNot();
    std::unique_ptr<ExpressionNode> parseEquality();
    std::unique_ptr<ExpressionNode> parseComparison();
    std::unique_ptr<ExpressionNode> parseTerm();
    std::unique_ptr<ExpressionNode> parseFactor();
    std::unique_ptr<ExpressionNode> parsePrimary();

private:
    std::unique_ptr<ASTNode> parseFunctionCall(const std::string& func_name);
    std::unique_ptr<ASTNode> parseAssignment(const std::string& var_name);
    std::unique_ptr<ASTNode> parseFunctionDefinition(const std::string& keyword);
    std::unique_ptr<ASTNode> parseKeywordFunctionCall(const std::string& keyword);
    std::unique_ptr<ASTNode> parseArrayLiteral(const std::string& name);
    std::unique_ptr<ASTNode> parserArrayAccess();
    std::unique_ptr<ASTNode> parseIfStatement();
    std::vector<std::unique_ptr<ASTNode>> parseElseChain();
    std::unique_ptr<ASTNode> parseUserFuncDefinition();
    std::unique_ptr<ASTNode> parseReturnStatement();
    std::unique_ptr<ASTNode> parseEveryStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::unique_ptr<ASTNode> parseForStatement();
    std::unique_ptr<ASTNode> parseRepeatStatement();
    std::unique_ptr<ASTNode> parseOnPressStatement();
    std::unique_ptr<ASTNode> parseControlTransferStatements(const std::string& statement);

public:
    Parser(const std::vector<Token>& tokens, const std::string& source);
    std::vector<std::unique_ptr<ASTNode>> parse();
};