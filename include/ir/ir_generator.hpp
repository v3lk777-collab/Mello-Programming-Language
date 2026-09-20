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
    std::vector<IRFunction> functions;
    IRFunction* currentFunction = nullptr;
    std::vector<IRInstruction> instructions;

private:
    int getID() noexcept;

private:
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
        std::cout << "ID: " << instruction.result.id << '\n';

        std::cout << "Opcode: ";

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

        default:
            std::cout << "UNKNOWN";
            break;
        }

        std::cout << '\n';

        std::cout << "Operands: ";

        for (const auto& operand : instruction.operands) {
            std::cout << "%" << operand.id << " ";
        }

        std::cout << '\n';

        std::cout << "Name: ";

        for (const auto& name : instruction.name) {
            std::cout << name;
        }

        std::cout << "\n\n";
    }

    void printInstructions() {
        std::cout << "Global:\n";

        for (const auto& instruction : global.instructions) {
            printInstruction(instruction);
        }

        for (const auto& function : functions) {
            std::cout << "Function: " << function.name << "\n";

            for (const auto& instruction : function.instructions) {
                printInstruction(instruction);
            }
        }
    }

private:
    IRValue generateVarAssignNode(VarAssignNode* varAssignNode);
    void generateFunctionNode(FunctionNode* functionNode);
    void generateUserFuncNode(UserFuncNode* userFuncNode);
    void generateSerialFunctionsCallNode(SerialFunctionsCallNode* serialFunctionsCallNode, MethodCallNode* methodCallNode);
    void generateBuiltInFunctionCallNode(BuiltInFunctionCallNode* builtInFunctionCallNode);
    void generateFunctionCallNode(FunctionCallNode* functionCallNode);
    void generateMethodCallNode(MethodCallNode* methodCallNode);

public:
    IRGenerator(const SymbolTable* symbolTable);

public:
    IRValue generateExpression(ASTNode* node);
    void generate(ASTNode* node);
};