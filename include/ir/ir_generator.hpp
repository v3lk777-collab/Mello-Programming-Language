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
#include "symbol_table.hpp"

#include <iostream>

class IRGenerator {
private:
    int currentID;
    IRFunction global;
    const SymbolTable* symbolTable;
    IRBlock* currentBlock = nullptr;
    std::vector<IRFunction> functions;
    IRFunction* currentFunction = nullptr;

public:
    int getID() noexcept;
    void setCurrentBlock(IRBlock block);

public:
    IRType mapTokenTypeToIRType(TokenType tokenType) noexcept;
    IRType mapTokenToIRType(Token token) noexcept;
    IRType mapDataTypeToIRType(DataType type) noexcept;
    IROpcode mapTokenToIROpcode(Token token) noexcept;

private:
    IRValue generateLiteralNode(LiteralNode* literalNode);
    IRValue generateBinaryOpNode(BinaryOpNode* binaryOpNode);

// Test
public:
    void printInstruction(const IRInstruction& instruction) {
        std::cout << "--> ID           : " << instruction.result.id << '\n';

        std::cout << "--> Opcode       : ";

        switch (instruction.opcode) {
        case IROpcode::CONSTANT:
            std::cout << "CONSTANT";
            break;

        case IROpcode::ADD:
            std::cout << "ADD";
            break;

        case IROpcode::SUB:
            std::cout << "SUB";
            break;

        case IROpcode::MUL:
            std::cout << "MUL";
            break;

        case IROpcode::DIV:
            std::cout << "DIV";
            break;

        case IROpcode::STORE:
            std::cout << "STORE";
            break;

        case IROpcode::CALL:
            std::cout << "CALL";
            break;

        case IROpcode::LOAD:
            std::cout << "LOAD";
            break;

        case IROpcode::RETURN:
            std::cout << "RETURN";
            break;

        case IROpcode::BRANCH:
            std::cout << "BRANCH";
            break;

        case IROpcode::CONDITIONAL_BRANCH:
            std::cout << "CONDITIONAL_BRANCH";
            break;

        case IROpcode::EQUAL:
            std::cout << "EQUAL";
            break;

        case IROpcode::EQUALITY:
            std::cout << "EQUALITY";
            break;

        case IROpcode::LESS:
            std::cout << "LESS";
            break;

        case IROpcode::GREATER:
            std::cout << "GREATER";
            break;

        case IROpcode::LESS_EQUAL:
            std::cout << "LESS_EQUAL";
            break;

        case IROpcode::GREATER_EQUAL:
            std::cout << "GREATER_EQUAL";
            break;

        case IROpcode::NOT_EQUAL:
            std::cout << "NOT_EQUAL";
            break;

        default:
            std::cout << "UNKNOWN";
            break;
        }

        std::cout << '\n';

        std::cout << "--> Operands     : ";

        for (const auto& operand : instruction.operands) {
            std::cout << "%" << operand.id << " ";
        }

        std::cout << '\n';

        std::cout << "--> Name         : ";

        for (const auto& name : instruction.name) {
            std::cout << name;
        }

        std::cout << '\n';

        std::cout << "--> Target       : ";

        for (const auto& target : instruction.target) {
            std::cout << target;
        }

        std::cout << '\n';

        std::cout << "--> False Target : ";

        for (const auto& falseTarget : instruction.falseTarget) {
            std::cout << falseTarget;
        }

        std::cout << "\n\n";
    }

    void printInstructions() {
        std::cout << "> Global:\n";

        for (const auto& block : global.blocks) {
            std::cout << "> Block: " << block.name << "\n";

            for (const auto& instruction : block.instructions) {
                printInstruction(instruction);
            }
        }

        for (const auto& function : functions) {
            std::cout << "> Function: " << function.name << "\n";

            for (const auto& block : function.blocks) {
                std::cout << "> Block: " << block.name << "\n";

                for (const auto& instruction : block.instructions) {
                    printInstruction(instruction);
                }
            }
        }
    }

private:
    void generateVarAssignNode(VarAssignNode* varAssignNode);
    void generateFunctionNode(FunctionNode* functionNode);
    void generateUserFuncNode(UserFuncNode* userFuncNode);
    void generateSerialFunctionsCallNode(SerialFunctionsCallNode* serialFunctionsCallNode, MethodCallNode* methodCallNode);
    IRValue generateBuiltInFunctionCallNode(BuiltInFunctionCallNode* builtInFunctionCallNode);
    IRValue generateFunctionCallNode(FunctionCallNode* functionCallNode);
    void generateMethodCallNode(MethodCallNode* methodCallNode);
    void generateReturnNode(ReturnNode* returnNode);
    void generateIfNode(IfNode* ifNode);
    void generateWhileNode(WhileNode* whileNode);
    void generateForNode(ForNode* forNode);
    IRValue generateVariableLoad(const std::string& name);
    void generateForRangeNode(ForRangeNode* forRangeNode);

public:
    IRGenerator(const SymbolTable* symbolTable);

public:
    IRValue generateExpression(ASTNode* node);

public:
    void generate(ASTNode* node);
};