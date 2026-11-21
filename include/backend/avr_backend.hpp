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

#include "visitor.hpp"

#include <sstream>

class AVRBackend : public ASTVisitor {
private:
    std::stringstream output;

private:
    std::string evaluateExpression(ExpressionNode* expression);
    std::string evaluateNode(ASTNode* node);

public:
    void visit(ArrayNode* arrayNode) override;
    void visit(ArrayIndexNode* arrayIndexNode) override;
    void visit(ArrayAssignNode* arrayAssignNode) override;
    void visit(ArrayBuiltInFunctionsNode* arrayBuiltInFunctionsNode) override;
    void visit(FunctionNode* functionNode) override;
    void visit(LiteralNode* literalNode) override;
    void visit(FunctionCallNode* functionCallNode) override;
    void visit(BuiltInFunctionCallNode* builtInFunctionCallNode) override;
    void visit(SerialFunctionsCallNode* serialFunctionsCallNode) override;
    void visit(MethodCallNode* methodCallNode) override;
    void visit(VarAssignNode* varAssignNode) override;
    void visit(IfNode* ifNode) override;
    void visit(ReturnNode* returnNode) override;
    void visit(UserFuncNode* userFuncNode) override;
    void visit(EveryNode* everyNode) override;
    void visit(WhileNode* whileNode) override;
    void visit(ForNode* forNode) override;
    void visit(ForRangeNode* forRangeNode) override;
    void visit(RepeatNode* repeatNode) override;
    void visit(OnPressNode* onPressNode) override;
    void visit(ControlTransferStatementsNode* controlTransferStatementsNode) override;
    void visit(ModuleNode* moduleNode) override;
    void visit(ClassNode* classNode) override;
    void visit(ObjectCreationNode* objectCreationNode) override;
    void visit(MemberAccessNode* memberAccessNode) override;
    void visit(MemberAssignNode* memberAssignNode) override;

public:
    void visit(UnaryOpNode* unaryOpNode) override;
    void visit(BinaryOpNode* binaryOpNode) override;
    void visit(BitwiseNode* bitwiseNode) override;
    void visit(CompoundAssignNode* compoundAssignNode) override;
    void visit(GroupNode* groupNode) override;
    void visit(TypeConversionCallNode* typeConversionCallNode) override;

public:
    std::string generate(const std::vector<std::unique_ptr<ASTNode>>& stdLibProgram, const std::vector<std::unique_ptr<ASTNode>>& program) override;
};