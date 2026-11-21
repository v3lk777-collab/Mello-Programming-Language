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

#include <string>
#include <vector>
#include <memory>

class ASTNode;
class ExpressionNode;

class ArrayNode;
class ArrayIndexNode;
class ArrayAssignNode;
class ArrayBuiltInFunctionsNode;
class FunctionNode;
class LiteralNode;
class FunctionCallNode;
class BuiltInFunctionCallNode;
class SerialFunctionsCallNode;
class MethodCallNode;
class VarAssignNode;
class IfNode;
class ReturnNode;
class UserFuncNode;
class EveryNode;
class WhileNode;
class ForNode;
class ForRangeNode;
class RepeatNode;
class OnPressNode;
class ControlTransferStatementsNode;
class ModuleNode;
class ClassNode;
class ObjectCreationNode;
class MemberAccessNode;
class MemberAssignNode;

class UnaryOpNode;
class BinaryOpNode;
class BitwiseNode;
class CompoundAssignNode;
class GroupNode;
class TypeConversionCallNode;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

public:
    virtual void visit(ArrayNode* arrayNode) = 0;
    virtual void visit(ArrayIndexNode* arrayIndexNode) = 0;
    virtual void visit(ArrayAssignNode* arrayAssignNode) = 0;
    virtual void visit(ArrayBuiltInFunctionsNode* arrayBuiltInFunctionsNode) = 0;
    virtual void visit(FunctionNode* functionNode) = 0;
    virtual void visit(LiteralNode* literalNode) = 0;
    virtual void visit(FunctionCallNode* functionCallNode) = 0;
    virtual void visit(BuiltInFunctionCallNode* builtInFunctionCallNode) = 0;
    virtual void visit(SerialFunctionsCallNode* serialFunctionsCallNode) = 0;
    virtual void visit(MethodCallNode* methodCallNode) = 0;
    virtual void visit(VarAssignNode* varAssignNode) = 0;
    virtual void visit(IfNode* ifNode) = 0;
    virtual void visit(ReturnNode* returnNode) = 0;
    virtual void visit(UserFuncNode* userFuncNode) = 0;
    virtual void visit(EveryNode* everyNode) = 0;
    virtual void visit(WhileNode* whileNode) = 0;
    virtual void visit(ForNode* forNode) = 0;
    virtual void visit(ForRangeNode* forRangeNode) = 0;
    virtual void visit(RepeatNode* repeatNode) = 0;
    virtual void visit(OnPressNode* onPressNode) = 0;
    virtual void visit(ControlTransferStatementsNode* controlTransferStatementsNode) = 0;
    virtual void visit(TypeConversionCallNode* typeConversionCallNode) = 0;
    virtual void visit(ModuleNode* moduleNode) = 0;
    virtual void visit(ClassNode* classNode) = 0;
    virtual void visit(ObjectCreationNode* objectCreationNode) = 0;
    virtual void visit(MemberAccessNode* memberAccessNode) = 0;
    virtual void visit(MemberAssignNode* memberAssignNode) = 0;

public:
    virtual void visit(UnaryOpNode* unaryOpNode) = 0;
    virtual void visit(BinaryOpNode* binaryOpNode) = 0;
    virtual void visit(BitwiseNode* bitwiseNode) = 0;
    virtual void visit(CompoundAssignNode* compoundAssignNode) = 0;
    virtual void visit(GroupNode* groupNode) = 0;

public:
    virtual std::string generate(const std::vector<std::unique_ptr<ASTNode>>& stdLibProgram, const std::vector<std::unique_ptr<ASTNode>>& program) = 0;
};