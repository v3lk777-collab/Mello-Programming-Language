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

#include "ir.hpp"
#include "ast.hpp"
#include "token.hpp"

class IRGenerator {
private:
    IROp mapTokenTypeToIROp(Token op);
    IRLiteralType mapTokenTypeToIRLiteralType(Token token);

private:
    std::unique_ptr<IRNode> generateUseStatement(UseNode* useNode);
    std::unique_ptr<IRNode> generateArrayLiteral(ArrayNode* arrayNode);
    std::unique_ptr<IRNode> generateArrayIndex(ArrayIndexNode* arrayIndexNode);
    std::unique_ptr<IRNode> generateArrayAssign(ArrayAssignNode* arrayAssignNode);
    std::unique_ptr<IRNode> generateFunction(FunctionNode* functionNode);
    std::unique_ptr<IRNode> generateLiteral(LiteralNode* literalNode);
    std::unique_ptr<IRNode> generateFunctionCall(FunctionCallNode* functionCallNode);
    std::unique_ptr<IRNode> generateBuiltInFunctionCall(BuiltInFunctionCallNode* builtInFunctionCallNode);
    std::unique_ptr<IRNode> generateSerialFunctionsCallNode(SerialFunctionsCallNode* serialFunctionsCallNode);
    std::unique_ptr<IRNode> generateMethodCall(MethodCallNode* methodCallNode);
    std::unique_ptr<IRNode> generateVarAssign(VarAssignNode* varAssignNode);
    std::unique_ptr<IRNode> generateUnaryOp(UnaryOpNode* unaryOpNode);
    std::unique_ptr<IRNode> generateBinaryOp(BinaryOpNode* binaryOpNode);
    std::unique_ptr<IRNode> generateCompoundAssign(CompoundAssignNode* compoundAssignNode);
    std::unique_ptr<IRNode> generateIfStatment(IfNode* ifNode);
    std::unique_ptr<IRNode> generateReturnStatement(ReturnNode* returnNode);
    std::unique_ptr<IRNode> generateUserFunc(UserFuncNode* userFuncNode);
    std::unique_ptr<IRNode> generateEveryStatement(EveryNode* everyNode);
    std::unique_ptr<IRNode> generateWhileStatement(WhileNode* whileNode);
    std::unique_ptr<IRNode> generateForStatement(ForNode* forNode);
    std::unique_ptr<IRNode> generateForRangeStatement(ForRangeNode* forRangeNode);
    std::unique_ptr<IRNode> generateRepeatStatement(RepeatNode* repeatNode);
    std::unique_ptr<IRNode> generateGroupStatement(GroupNode* groupNode);
    std::unique_ptr<IRNode> generateOnPressStatement(OnPressNode* onPressNode);
    std::unique_ptr<IRNode> generateControlTransferStatements(ControlTransferStatementsNode* controlTransferStatementsNode);
    std::unique_ptr<IRNode> generateTypeConversionCall(TypeConversionCallNode* typeConversionCallNode);

public:
    std::unique_ptr<IRNode> generateSingle(ASTNode* node);
    std::vector<std::unique_ptr<IRNode>> generate(ASTNode* node);
};