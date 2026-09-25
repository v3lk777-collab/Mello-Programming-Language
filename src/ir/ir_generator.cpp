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
    global.name = "global";

    currentFunction = &global;

    IRBlock globalEntryBlock;

    globalEntryBlock.name = "Global Block";

    setCurrentBlock(globalEntryBlock);
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

    currentBlock->instructions.push_back(instruction);

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

    currentBlock->instructions.push_back(instruction);

    return result;
}

IRValue IRGenerator::generateExpression(ASTNode* node) {
    if (auto* literalNode = dynamic_cast<LiteralNode*>(node)) {
        return generateLiteralNode(literalNode);
    } else if (auto* binaryOpNode = dynamic_cast<BinaryOpNode*>(node)) {
        return generateBinaryOpNode(binaryOpNode);
    } else if (auto* builtInFunctionCallNode = dynamic_cast<BuiltInFunctionCallNode*>(node)) {
        return generateBuiltInFunctionCallNode(builtInFunctionCallNode);
    } else if (auto* functionCallNode = dynamic_cast<FunctionCallNode*>(node)) {
        return generateFunctionCallNode(functionCallNode);
    }

    return {};
}

void IRGenerator::generateVarAssignNode(VarAssignNode* varAssignNode) {
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

    currentBlock->instructions.push_back(instruction);
}

void IRGenerator::generateFunctionNode(FunctionNode* functionNode) {
    IRFunction function;

    function.name = functionNode->getFunctionName();

    functions.push_back(function);
    currentFunction = &functions.back();

    IRBlock entryBlock;

    entryBlock.name = "Block " + std::to_string(getID());

    setCurrentBlock(entryBlock);

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

    IRBlock entryBlock;

    entryBlock.name = "Block " + std::to_string(getID());

    setCurrentBlock(entryBlock);

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

    currentBlock->instructions.push_back(instruction);
}

void IRGenerator::generateMethodCallNode(MethodCallNode* methodCallNode) {
    if (methodCallNode->getObjectName() == "serial") {
        if (auto* serialFunctionsCallNode = dynamic_cast<SerialFunctionsCallNode*>(methodCallNode->getMethodCall())) {
            generateSerialFunctionsCallNode(serialFunctionsCallNode, methodCallNode);
        }
    }
}

IRValue IRGenerator::generateBuiltInFunctionCallNode(BuiltInFunctionCallNode* builtInFunctionCallNode) {
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

    currentBlock->instructions.push_back(instruction);

    return result;
}

IRValue IRGenerator::generateFunctionCallNode(FunctionCallNode* functionCallNode) {
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

    currentBlock->instructions.push_back(instruction);

    return result;
}

void IRGenerator::generateReturnNode(ReturnNode* returnNode) {
    IRValue value = generateExpression(returnNode->getValue());

    IRInstruction instruction;

    instruction.result.id = getID();
    instruction.result.type = value.type;
    instruction.result.isConstant = false;

    instruction.opcode = IROpcode::RETURN;
    instruction.operands = { value };

    currentBlock->instructions.push_back(instruction);
}

void IRGenerator::generateIfNode(IfNode* ifNode) {
    IRBlock endBlock;
    IRBlock thenBlock;
    IRBlock elseBlock;
    IRBlock conditionBlock;

    conditionBlock.name = "Block "  + std::to_string(getID());
    endBlock.name = "Block "  + std::to_string(getID());
    thenBlock.name = "Block "  + std::to_string(getID());
    elseBlock.name = "Block "  + std::to_string(getID());

    setCurrentBlock(conditionBlock);

    IRValue condition = generateExpression(ifNode->getCondition().get());

    IRInstruction branch;

    branch.result.id = getID();
    branch.opcode = IROpcode::CONDITIONAL_BRANCH;
    branch.target = thenBlock.name;
    branch.falseTarget = elseBlock.name;
    branch.operands = { condition };

    currentBlock->instructions.push_back(branch);

    setCurrentBlock(thenBlock);

    for (const auto& node : ifNode->getThenBody()) {
        generate(node.get());
    }

    IRInstruction thenJump;

    thenJump.result.id = getID();
    thenJump.opcode = IROpcode::BRANCH;
    thenJump.target = endBlock.name;

    currentBlock->instructions.push_back(thenJump);

    setCurrentBlock(elseBlock);

    for (const auto& node : ifNode->getElseBody()) {
        generate(node.get());
    }

    IRInstruction elseJump;

    elseJump.result.id = getID();
    elseJump.opcode = IROpcode::BRANCH;
    elseJump.target = endBlock.name;

    currentBlock->instructions.push_back(elseJump);

    setCurrentBlock(endBlock);
}

void IRGenerator::generateWhileNode(WhileNode* whileNode) {
    IRBlock conditionBlock;
    IRBlock loopBlock;
    IRBlock endBlock;

    conditionBlock.name = "Block " + std::to_string(getID());
    loopBlock.name = "Block " + std::to_string(getID());
    endBlock.name = "Block " + std::to_string(getID());

    setCurrentBlock(conditionBlock);

    IRValue condition = generateExpression(whileNode->getCondition().get());

    IRInstruction branch;

    branch.result.id = getID();
    branch.target = loopBlock.name;
    branch.falseTarget = endBlock.name;
    branch.opcode = IROpcode::CONDITIONAL_BRANCH;
    branch.operands = { condition };

    currentBlock->instructions.push_back(branch);

    setCurrentBlock(loopBlock);

    for (const auto& node : whileNode->getBody()) {
        generate(node.get());
    }

    IRInstruction loopJump;

    loopJump.result.id = getID();
    loopJump.target = conditionBlock.name;
    loopJump.opcode = IROpcode::BRANCH;

    currentBlock->instructions.push_back(loopJump);

    setCurrentBlock(endBlock);
}

void IRGenerator::generateForNode(ForNode* forNode) {
    IRBlock conditionBlock;
    IRBlock loopBlock;
    IRBlock endBlock;

    conditionBlock.name = "Block " + std::to_string(getID());
    loopBlock.name = "Block " + std::to_string(getID());
    endBlock.name = "Block " + std::to_string(getID());

    setCurrentBlock(conditionBlock);

    IRValue condition = generateExpression(forNode->getCondition().get());

    IRInstruction branch;

    branch.result.id = getID();
    branch.target = loopBlock.name;
    branch.falseTarget = endBlock.name;
    branch.opcode = IROpcode::CONDITIONAL_BRANCH;
    branch.operands = { condition };

    currentBlock->instructions.push_back(branch);

    setCurrentBlock(loopBlock);

    for (const auto& node : forNode->getBody()) {
        generate(node.get());
    }

    IRInstruction loopJump;

    loopJump.result.id = getID();
    loopJump.target = conditionBlock.name;
    loopJump.opcode = IROpcode::BRANCH;

    currentBlock->instructions.push_back(loopJump);

    setCurrentBlock(endBlock);
}

IRValue IRGenerator::generateVariableLoad(const std::string& name) {
    const VariableSymbol* var = symbolTable->lookupVariable(name);

    IRValue result;

    result.id = getID();
    result.value = name;
    result.type = var ? mapDataTypeToIRType(var->type) : IRType::VOID;
    result.isConstant = false;

    IRInstruction instruction;

    instruction.result = result;
    instruction.name = name;
    instruction.opcode = IROpcode::LOAD;

    currentBlock->instructions.push_back(instruction);

    return result;
}

void IRGenerator::generateForRangeNode(ForRangeNode* forRangeNode) {
    IRBlock initBlock;
    IRBlock conditionBlock;
    IRBlock bodyBlock;
    IRBlock incrementBlock;
    IRBlock endBlock;

    initBlock.name = "Block " + std::to_string(getID());
    conditionBlock.name = "Block " + std::to_string(getID());
    bodyBlock.name = "Block " + std::to_string(getID());
    incrementBlock.name = "Block " + std::to_string(getID());
    endBlock.name = "Block " + std::to_string(getID());

    setCurrentBlock(initBlock);

    IRValue start = generateExpression(forRangeNode->getStart().get());

    IRInstruction initStore;

    initStore.result.id = getID();
    initStore.opcode = IROpcode::STORE;
    initStore.name = forRangeNode->getVarName();
    initStore.operands = { start };

    currentBlock->instructions.push_back(initStore);

    IRInstruction initJump;

    initJump.result.id = getID();
    initJump.opcode = IROpcode::BRANCH;
    initJump.target = conditionBlock.name;

    currentBlock->instructions.push_back(initJump);

    setCurrentBlock(conditionBlock);

    IRValue variable = generateVariableLoad(forRangeNode->getVarName());

    IRValue stop = generateExpression(forRangeNode->getStop().get());

    IRInstruction compare;

    compare.result.id = getID();
    compare.result.type = IRType::BOOLEAN;
    compare.opcode = IROpcode::LESS;
    compare.operands = { variable, stop };

    currentBlock->instructions.push_back(compare);

    IRInstruction conditionBranch;

    conditionBranch.result.id = getID();
    conditionBranch.opcode = IROpcode::CONDITIONAL_BRANCH;
    conditionBranch.operands = { compare.result };
    conditionBranch.target = bodyBlock.name;
    conditionBranch.falseTarget = endBlock.name;

    currentBlock->instructions.push_back(conditionBranch);

    setCurrentBlock(bodyBlock);

    for (const auto& node : forRangeNode->getBody()) {
        generate(node.get());
    }

    IRInstruction bodyJump;

    bodyJump.result.id = getID();
    bodyJump.opcode = IROpcode::BRANCH;
    bodyJump.target = incrementBlock.name;

    currentBlock->instructions.push_back(bodyJump);

    setCurrentBlock(incrementBlock);

    IRValue currentValue = generateVariableLoad(forRangeNode->getVarName());

    IRValue step = generateExpression(forRangeNode->getStep().get());

    IRInstruction add;

    add.result.id = getID();
    add.result.type = currentValue.type;
    add.opcode = IROpcode::ADD;
    add.operands = { currentValue, step };

    currentBlock->instructions.push_back(add);

    IRInstruction incrementStore;

    incrementStore.result.id = getID();
    incrementStore.opcode = IROpcode::STORE;
    incrementStore.name = forRangeNode->getVarName();
    incrementStore.operands = { add.result };

    currentBlock->instructions.push_back(incrementStore);

    IRInstruction incrementJump;

    incrementJump.result.id = getID();
    incrementJump.opcode = IROpcode::BRANCH;
    incrementJump.target = conditionBlock.name;

    currentBlock->instructions.push_back(incrementJump);

    setCurrentBlock(endBlock);
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
    } else if (auto* returnNode = dynamic_cast<ReturnNode*>(node)) {
        generateReturnNode(returnNode);
    } else if (auto* ifNode = dynamic_cast<IfNode*>(node)) {
        generateIfNode(ifNode);
    } else if (auto* whileNode = dynamic_cast<WhileNode*>(node)) {
        generateWhileNode(whileNode);
    } else if (auto* forNode = dynamic_cast<ForNode*>(node)) {
        generateForNode(forNode);
    } else if (auto* forRangeNode = dynamic_cast<ForRangeNode*>(node)) {
        generateForRangeNode(forRangeNode);
    }
}