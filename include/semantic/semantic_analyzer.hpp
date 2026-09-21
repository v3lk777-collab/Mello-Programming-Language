/*
 * Mello Programming Language
 *
 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
 *
 * This source code is proprietary and confidential. Unauthorized copying, 
 * modification, distribution, or use of this file for any academic, 
 * commercial, or competitive purpose, via any medium, is strictly 
 * prohibited without the express written permission of the author.
 */

#pragma once

#include "ast.hpp"
#include "symbol_table.hpp"

#include <unordered_set>

const std::unordered_set<std::string> builtInLibraries = {
    "Servo", "Wire", "SPI", "math"
};

class SemanticAnalyzer {
private:
    SymbolTable symbolTable;

public:
    DataType mapStringToDataType(const std::string& typeStr);

private:
    void analyzeAssignment(VarAssignNode* varNode);
    void analyzeIfStatment(IfNode* ifNode);
    void analyzeUserFuncDefinition(UserFuncNode* userFuncNode);
    void analyzeWhileStatement(WhileNode* whileNode);
    void analyzeForStatement(ForNode* forNode);
    void analyzeForRangeStatement(ForRangeNode* forRangeNode);
    void analyzeEveryStatement(EveryNode* everyNode);
    void analyzeUseStatement(UseNode* useNode);
    void analyzeRepeatStatement(RepeatNode* repeatNode);
    void analyzeOnPressStatement(OnPressNode* onPressNode);
    void analyzeFunctionDefinition(FunctionNode* funcNode);

private:
    void analyzeLiteral(LiteralNode* literalNode);
    void analyzeBinaryOp(BinaryOpNode* binaryOpNode);
    void analyzeUnaryOp(UnaryOpNode* unaryOpNode);
    void analyzeGroup(GroupNode* groupNode);
    void analyzeFunctionCall(FunctionCallNode* functionCallNode);
    void analyzeMethodCall(MethodCallNode* methodCallNode);
    void analyzeSerialCall(SerialFunctionsCallNode* serialCallNode);
    void analyzeBuiltInCall(BuiltInFunctionCallNode* builtInCallNode);

public:
    void analyzeNode(ASTNode* node);
    void analyzeExpression(ExpressionNode* node);

public:
    const SymbolTable& getSymbolTable() const {
        return symbolTable;
    }
};