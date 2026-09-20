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

#include "ir_generator.hpp"

#include "error_handler.hpp"

IRGenerator::IRGenerator(const SymbolTable* symbolTable) : symbolTable(symbolTable), currentID(0) {
    instructions.clear();

    global.name = "global";

    currentFunction = &global;
}

int IRGenerator::getID() noexcept {
    return currentID++;
}

IRType IRGenerator::mapTokenTypeToIRType(TokenType tokenType) noexcept {
    switch (tokenType) {
    case TokenType::INTEGER:
        return IRType::INTEGER;

    case TokenType::FLOAT:
        return IRType::FLOAT;

    case TokenType::STRING:
        return IRType::STRING;

    case TokenType::CHARACTER:
        return IRType::CHARACTER;

    case TokenType::BOOLEAN:
        return IRType::BOOLEAN;

    default:
        return IRType::VOID;
    }
}

IRType IRGenerator::mapTokenToIRType(Token token) noexcept {
    switch (token.type) {
    case TokenType::INTEGER:
        return IRType::INTEGER;

    case TokenType::FLOAT:
        return IRType::FLOAT;

    case TokenType::STRING:
        return IRType::STRING;

    case TokenType::CHARACTER:
        return IRType::CHARACTER;

    case TokenType::BOOLEAN:
        return IRType::BOOLEAN;

    default:
        return IRType::VOID;
    }
}

IRType IRGenerator::mapDataTypeToIRType(DataType type) noexcept {
    switch (type) {
    case DataType::INTEGER:
        return IRType::INTEGER;

    case DataType::FLOAT:
        return IRType::FLOAT;

    case DataType::STRING:
        return IRType::STRING;

    case DataType::BOOLEAN:
        return IRType::BOOLEAN;

    default:
        return IRType::VOID;
    }
}

IROpcode IRGenerator::mapTokenToIROpcode(Token token) noexcept {
    switch (token.type) {
    case TokenType::PLUS:
        return IROpcode::ADD;

    case TokenType::MINUS:
        return IROpcode::SUB;

    case TokenType::MULTIPLY:
        return IROpcode::MUL;

    case TokenType::DIVIDE:
        return IROpcode::DIV;

    default:
        return IROpcode::CONSTANT;
    }
}

IRValue IRGenerator::generateLiteralNode(LiteralNode* literalNode) {
    const Token& token = literalNode->getToken();

    IRValue result;
    IRInstruction instruction;

    result.id = getID();
    result.value = token.value;

    if (token.type == TokenType::IDENTIFIER) {
        const VariableSymbol* var = symbolTable->lookupVariable(token.value);

        result.type = (var != nullptr) ? mapDataTypeToIRType(var->type) : IRType::VOID;
        result.isConstant = false;

        instruction.result = result;
        instruction.name = token.value;
        instruction.opcode = IROpcode::LOAD;
    } else {
        result.type = mapTokenToIRType(token);
        result.isConstant = true;

        instruction.result = result;
        instruction.opcode = IROpcode::CONSTANT;
    }

    currentFunction->instructions.push_back(instruction);

    return result;
}

IRValue IRGenerator::generateBinaryOpNode(BinaryOpNode* binaryOpNode) {
    IRValue left = generateExpression(binaryOpNode->getLeft());
    IRValue right = generateExpression(binaryOpNode->getRight());

    IRValue result;

    result.id = getID();

    auto typeInference = [&]() -> IRType {
        if (left.type == IRType::INTEGER && right.type == IRType::INTEGER) {
            return IRType::INTEGER;
        } else if (left.type == IRType::FLOAT && right.type == IRType::FLOAT) {
            return IRType::FLOAT;
        } else if (left.type == IRType::STRING && right.type == IRType::STRING) {
            return IRType::STRING;
        } else if (left.type == IRType::BOOLEAN && right.type == IRType::BOOLEAN) {
            return IRType::BOOLEAN;
        } else if (left.type == IRType::CHARACTER && right.type == IRType::CHARACTER) {
            return IRType::CHARACTER;
        } else {
            return IRType::VOID;
        }
    };

    result.type = typeInference();
    result.isConstant = false;

    IRInstruction instruction;

    instruction.result = result;
    instruction.opcode = mapTokenToIROpcode(binaryOpNode->getOp());
    instruction.operands = { left, right };

    currentFunction->instructions.push_back(instruction);

    return result;
}

IRValue IRGenerator::generateExpression(ASTNode* node) {
    if (auto* literalNode = dynamic_cast<LiteralNode*>(node)) {
        return generateLiteralNode(literalNode);
    }

    if (auto* binaryOpNode = dynamic_cast<BinaryOpNode*>(node)) {
        return generateBinaryOpNode(binaryOpNode);
    }

    return {};
}

IRValue IRGenerator::generateVarAssignNode(VarAssignNode* varAssignNode) {
    IRValue result;

    result.type = mapTokenTypeToIRType(varAssignNode->getValueType());
    result.isConstant = true;

    IRValue value = generateExpression(varAssignNode->getValue());

    IRInstruction instruction;

    result.id = getID();

    instruction.result = result;
    instruction.name = varAssignNode->getName();
    instruction.opcode = IROpcode::STORE;
    instruction.operands = { value };

    currentFunction->instructions.push_back(instruction);

    return result;
}

void IRGenerator::generateFunctionNode(FunctionNode* functionNode) {
    IRFunction function;

    function.name = functionNode->getFunctionName();

    functions.push_back(function);
    currentFunction = &functions.back();

    for (const auto& node : functionNode->getBody()) {
        generate(node.get());
    }

    currentFunction = &global;
}

void IRGenerator::generateUserFuncNode(UserFuncNode* userFuncNode) {
    IRFunction function;

    function.name = userFuncNode->getFunctionName();
    function.parameters = userFuncNode->getFuncParams();

    functions.push_back(function);
    currentFunction = &functions.back();

    for (const auto& node : userFuncNode->getBody()) {
        generate(node.get());
    }

    currentFunction = &global;
}

void IRGenerator::generateSerialFunctionsCallNode(SerialFunctionsCallNode* serialFunctionsCallNode, MethodCallNode* methodCallNode) {
    std::vector<IRValue> arguments;

    for (const auto& argument : serialFunctionsCallNode->getArguments()) {
        arguments.push_back(generateExpression(argument.get()));
    }

    IRValue result;

    result.id = getID();
    result.type = IRType::VOID;

    IRInstruction instruction;

    instruction.result = result;
    instruction.name = methodCallNode->getObjectName() + "." + serialFunctionsCallNode->getFunctionName();
    instruction.opcode = IROpcode::CALL;
    instruction.operands = arguments;

    currentFunction->instructions.push_back(instruction);
}

void IRGenerator::generateMethodCallNode(MethodCallNode* methodCallNode) {
    if (methodCallNode->getObjectName() == "serial") {
        if (auto* serialFunctionsCallNode = dynamic_cast<SerialFunctionsCallNode*>(methodCallNode->getMethodCall())) {
            generateSerialFunctionsCallNode(serialFunctionsCallNode, methodCallNode);
        }
    }
}

void IRGenerator::generateBuiltInFunctionCallNode(BuiltInFunctionCallNode* builtInFunctionCallNode) {
    std::vector<IRValue> arguments;

    for (const auto& argument : builtInFunctionCallNode->getArguments()) {
        arguments.push_back(generateExpression(argument.get()));
    }

    IRValue result;

    result.id = getID();
    result.type = IRType::VOID;

    IRInstruction instruction;

    instruction.result = result;
    instruction.name = builtInFunctionCallNode->getFunctionName();
    instruction.opcode = IROpcode::CALL;
    instruction.operands = arguments;

    currentFunction->instructions.push_back(instruction);
}

void IRGenerator::generateFunctionCallNode(FunctionCallNode* functionCallNode) {
    std::vector<IRValue> arguments;

    for (const auto& argument : functionCallNode->getArguments()) {
        arguments.push_back(generateExpression(argument.get()));
    }

    IRValue result;

    result.id = getID();
    result.type = IRType::VOID;

    IRInstruction instruction;

    instruction.result = result;
    instruction.name = functionCallNode->getFunctionName();
    instruction.opcode = IROpcode::CALL;
    instruction.operands = arguments;

    currentFunction->instructions.push_back(instruction);
}

void IRGenerator::generate(ASTNode* node) {
    if (auto* varAssignNode = dynamic_cast<VarAssignNode*>(node)) {
        generateVarAssignNode(varAssignNode);
    } else if (auto* functionNode = dynamic_cast<FunctionNode*>(node)) {
        generateFunctionNode(functionNode);
    } else if (auto* userFunctionNode = dynamic_cast<UserFuncNode*>(node)) {
        generateUserFuncNode(userFunctionNode);
    } else if (auto* methodCallNode = dynamic_cast<MethodCallNode*>(node)) {
        generateMethodCallNode(methodCallNode);
    } else if (auto* builtInFunctionCallNode = dynamic_cast<BuiltInFunctionCallNode*>(node)) {
        generateBuiltInFunctionCallNode(builtInFunctionCallNode);
    } else if (auto* functionCallNode = dynamic_cast<FunctionCallNode*>(node)) {
        generateFunctionCallNode(functionCallNode);
    }
}