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

IROp IRGenerator::mapTokenTypeToIROp(Token op) {
    std::string opValue = op.value;

    IROp type;

    if (opValue == "+" || opValue == "+=") {
        type = IROp::PLUS;
    }

    if (opValue == "-" || opValue == "-=") {
        type = IROp::MINUS;
    }

    if (opValue == "*" || opValue == "*=") {
        type = IROp::MULTIPLY;
    }

    if (opValue == "/" || opValue == "/=") {
        type = IROp::DIVIDE;
    }

    if (opValue == "%" || opValue == "%=") {
        type = IROp::MODULO;
    }

    if (opValue == "++") {
        type = IROp::INCREMENT;
    }

    if (opValue == "--") {
        type = IROp::DECREMENT;
    }

    return type;
}

IRLiteralType IRGenerator::mapTokenTypeToIRLiteralType(Token token) {
    IRLiteralType type;

    switch (token.type) {
    case TokenType::INTEGER:
        type = IRLiteralType::INTEGER;
        break;

    case TokenType::FLOAT:
        type = IRLiteralType::FLOAT;
        break;

    case TokenType::STRING:
        type = IRLiteralType::STRING;
        break;

    case TokenType::CHARACTER:
        type = IRLiteralType::CHARACTER;
        break;

    case TokenType::BOOLEAN:
        type = IRLiteralType::BOOLEAN;
        break;
    }

    return type;
}

std::unique_ptr<IRNode> IRGenerator::generateUseStatement(UseNode* useNode) {
    std::string libraryName = useNode->getLibraryName();

    return std::make_unique<IRUseNode>(std::move(libraryName));
}

std::unique_ptr<IRNode> IRGenerator::generateArrayLiteral(ArrayNode* arrayNode) {
    std::string arrayName = arrayNode->getArrayName();
    std::vector<std::string> arrayMembers = arrayNode->getArrayMembers();

    return std::make_unique<IRArrayNode>(std::move(arrayName), std::move(arrayMembers));
}

std::unique_ptr<IRNode> IRGenerator::generateArrayIndex(ArrayIndexNode* arrayIndexNode) {
    std::string arrayName = arrayIndexNode->getArrayName();
    std::unique_ptr<IRNode> indexExpression = generateSingle(arrayIndexNode->getIndexExpression().get());

    return std::make_unique<IRArrayIndexNode>(std::move(arrayName), std::move(indexExpression));
}

std::unique_ptr<IRNode> IRGenerator::generateArrayAssign(ArrayAssignNode* arrayAssignNode) {
    std::string arrayName = arrayAssignNode->getArrayName();
    std::unique_ptr<IRNode> indexExpression = generateSingle(arrayAssignNode->getIndexExpression().get());
    std::unique_ptr<IRNode> valueExpression = generateSingle(arrayAssignNode->getValueExpression().get());

    return std::make_unique<IRArrayAssignNode>(std::move(arrayName), std::move(indexExpression), std::move(valueExpression));
}

std::unique_ptr<IRNode> IRGenerator::generateFunction(FunctionNode* functionNode) {
    std::string functionName = functionNode->getFunctionName();

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto& stmt : functionNode->getBody()) {
        body.push_back(generateSingle(stmt.get()));
    }

    return std::make_unique<IRFunctionNode>(std::move(functionName), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateLiteral(LiteralNode* literalNode) {
    const Token& token = literalNode->getToken();

    std::string value = token.value;
    IRLiteralType type = mapTokenTypeToIRLiteralType(token);

    return std::make_unique<IRLiteralNode>(std::move(value), type);
}

std::unique_ptr<IRNode> IRGenerator::generateFunctionCall(FunctionCallNode* functionCallNode) {
    std::string functionName = functionCallNode->getFuncName();

    std::vector<std::unique_ptr<IRNode>> arguments;
    for (const auto& arg : functionCallNode->getArguments()) {
        arguments.push_back(generateSingle(arg.get()));
    }

    return std::make_unique<IRFunctionCallNode>(std::move(functionName), std::move(arguments));
}

std::unique_ptr<IRNode> IRGenerator::generateBuiltInFunctionCall(BuiltInFunctionCallNode* builtInFunctionCallNode) {
    std::string functionName = builtInFunctionCallNode->getFuncName();

    std::vector<std::unique_ptr<IRNode>> arguments;
    for (const auto& arg : builtInFunctionCallNode->getArguments()) {
        arguments.push_back(generateSingle(arg.get()));
    }

    return std::make_unique<IRBuiltInFunctionCallNode>(std::move(functionName), std::move(arguments));
}

std::unique_ptr<IRNode> IRGenerator::generateSerialFunctionsCallNode(SerialFunctionsCallNode* serialFunctionsCallNode) {
    std::string functionName = serialFunctionsCallNode->getFunctionName();

    std::vector<std::unique_ptr<IRNode>> arguments;
    for (const auto& arg : serialFunctionsCallNode->getArguments()) {
        arguments.push_back(generateSingle(arg.get()));
    }

    return std::make_unique<IRSerialFunctionsCallNode>(std::move(functionName), std::move(arguments));
}

std::unique_ptr<IRNode> IRGenerator::generateMethodCall(MethodCallNode* methodCallNode) {
    std::string methodObjectName = methodCallNode->getObjectName();
    std::unique_ptr<IRNode> methodCall = generateSingle(methodCallNode->getMethodCall());

    return std::make_unique<IRMethodCallNode>(std::move(methodObjectName), std::move(methodCall));
}

std::unique_ptr<IRNode> IRGenerator::generateVarAssign(VarAssignNode* varAssignNode) {
    std::string name = varAssignNode->name;
    bool isConstantVar = varAssignNode->isConstantVar;
    std::unique_ptr<IRNode> value = generateSingle(varAssignNode->value.get());

    return std::make_unique<IRVarAssignNode>(std::move(name), std::move(value), isConstantVar);
}

std::unique_ptr<IRNode> IRGenerator::generateUnaryOp(UnaryOpNode* unaryOpNode) {
    const Token& token = unaryOpNode->getOp();

    IROp op = mapTokenTypeToIROp(token);
    std::unique_ptr<IRNode> value = generateSingle(unaryOpNode->getRight());

    return std::make_unique<IRUnaryOpNode>(op, std::move(value));
}

std::unique_ptr<IRNode> IRGenerator::generateBinaryOp(BinaryOpNode* binaryOpNode) {
    const Token& token = binaryOpNode->getOp();

    IROp op = mapTokenTypeToIROp(token);
    std::unique_ptr<IRNode> left = generateSingle(binaryOpNode->getLeft());
    std::unique_ptr<IRNode> right = generateSingle(binaryOpNode->getRight());

    return std::make_unique<IRBinaryOpNode>(std::move(left), op, std::move(right));
}

std::unique_ptr<IRNode> IRGenerator::generateCompoundAssign(CompoundAssignNode* compoundAssignNode) {
    std::string opValue = compoundAssignNode->getOp();
    std::string name = compoundAssignNode->getName();
    std::string value = compoundAssignNode->getValue();

    IROp op = mapTokenTypeToIROp(Token{opValue, TokenType::PLUS});

    return std::make_unique<IRCompoundAssignNode>(std::move(name), op, std::move(value));
}

std::unique_ptr<IRNode> IRGenerator::generateIfStatment(IfNode* ifNode) {
    std::unique_ptr<IRNode> condition = generateSingle(ifNode->getCondition().get());

    std::vector<std::unique_ptr<IRNode>> thenBody;
    for (const auto& stmt : ifNode->getThenBody()) {
        thenBody.push_back(generateSingle(stmt.get()));
    }

    std::vector<std::unique_ptr<IRNode>> elseBody;
    for (const auto& stmt : ifNode->getElseBody()) {
        elseBody.push_back(generateSingle(stmt.get()));
    }

    return std::make_unique<IRIfNode>(std::move(condition), std::move(thenBody), std::move(elseBody));
}

std::unique_ptr<IRNode> IRGenerator::generateReturnStatement(ReturnNode* returnNode) {
    std::unique_ptr<IRNode> value = generateSingle(returnNode->getValue());

    return std::make_unique<IRReturnNode>(std::move(value));
}

std::unique_ptr<IRNode> IRGenerator::generateUserFunc(UserFuncNode* userFuncNode) {
    std::string funcName = userFuncNode->getFuncName();
    std::vector<std::string> params = userFuncNode->getFuncParams();

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto& stmt : userFuncNode->getFuncBody()) {
        body.push_back(generateSingle(stmt.get()));
    }

    return std::make_unique<IRUserFuncNode>(std::move(funcName), std::move(params), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateEveryStatement(EveryNode* everyNode) {
    int id = everyNode->getId();
    std::string interval = everyNode->getInterval();

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto* childBody : everyNode->getChildBodies()) {
        for (const auto& stmt : *childBody) {
            body.push_back(generateSingle(stmt.get()));
        }
    }

    return std::make_unique<IREveryNode>(id, std::move(interval), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateWhileStatement(WhileNode* whileNode) {
    std::unique_ptr<IRNode> condition = generateSingle(whileNode->getCondition().get());

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto* childBody : whileNode->getChildBodies()) {
        for (const auto& stmt : *childBody) {
            body.push_back(generateSingle(stmt.get()));
        }
    }

    return std::make_unique<IRWhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateForStatement(ForNode* forNode) {
    std::unique_ptr<IRNode> condition = generateSingle(forNode->getCondition().get());

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto* childBody : forNode->getChildBodies()) {
        for (const auto& stmt : *childBody) {
            body.push_back(generateSingle(stmt.get()));
        }
    }

    return std::make_unique<IRForNode>(std::move(condition), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateForRangeStatement(ForRangeNode* forRangeNode) {
    std::string varName = forRangeNode->getVarName();
    std::unique_ptr<IRNode> start = generateSingle(forRangeNode->getStart().get());
    std::unique_ptr<IRNode> stop = generateSingle(forRangeNode->getStop().get());
    std::unique_ptr<IRNode> step = generateSingle(forRangeNode->getStep().get());

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto* childBody : forRangeNode->getChildBodies()) {
        for (const auto& stmt : *childBody) {
            body.push_back(generateSingle(stmt.get()));
        }
    }

    return std::make_unique<IRForRangeNode>(std::move(varName), std::move(start), std::move(stop), std::move(step), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateRepeatStatement(RepeatNode* repeatNode) {
    int id = repeatNode->getId();
    std::string count = repeatNode->getCount();

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto* childBody : repeatNode->getChildBodies()) {
        for (const auto& stmt : *childBody) {
            body.push_back(generateSingle(stmt.get()));
        }
    }

    return std::make_unique<IRRepeatNode>(id, std::move(count), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateGroupStatement(GroupNode* groupNode) {
    std::unique_ptr<IRNode> expression = generateSingle(groupNode->getExpression());

    return std::make_unique<IRGroupNode>(std::move(expression));
}

std::unique_ptr<IRNode> IRGenerator::generateOnPressStatement(OnPressNode* onPressNode) {
    int id = onPressNode->getId();
    std::string pin = onPressNode->getPin();

    std::vector<std::unique_ptr<IRNode>> body;
    for (const auto* childBody : onPressNode->getChildBodies()) {
        for (const auto& stmt : *childBody) {
            body.push_back(generateSingle(stmt.get()));
        }
    }

    return std::make_unique<IROnPressNode>(id, std::move(pin), std::move(body));
}

std::unique_ptr<IRNode> IRGenerator::generateControlTransferStatements(ControlTransferStatementsNode* controlTransferStatementsNode) {
    std::string statement = controlTransferStatementsNode->getStatement();

    return std::make_unique<IRControlTransferStatementsNode>(std::move(statement));
}

std::unique_ptr<IRNode> IRGenerator::generateTypeConversionCall(TypeConversionCallNode* typeConversionCallNode) {
    std::string funcName = typeConversionCallNode->getFunctionName();

    std::vector<std::unique_ptr<IRNode>> arguments;
    for (const auto& arg : typeConversionCallNode->getArguments()) {
        arguments.push_back(generateSingle(arg.get()));
    }

    return std::make_unique<IRTypeConversionCallNode>(std::move(funcName), std::move(arguments));
}

std::unique_ptr<IRNode> IRGenerator::generateSingle(ASTNode* node) {
    std::vector<std::unique_ptr<IRNode>> nodes = generate(node);

    if (nodes.empty()) {
        return nullptr;
    }

    return std::move(nodes.front());
}

std::vector<std::unique_ptr<IRNode>> IRGenerator::generate(ASTNode* node) {
    std::vector<std::unique_ptr<IRNode>> ir;

    if (auto* useNode = dynamic_cast<UseNode*>(node)) {
        ir.push_back(generateUseStatement(useNode));
    } else if (auto* arrayNode = dynamic_cast<ArrayNode*>(node)) {
        ir.push_back(generateArrayLiteral(arrayNode));
    } else if (auto* arrayIndexNode = dynamic_cast<ArrayIndexNode*>(node)) {
        ir.push_back(generateArrayIndex(arrayIndexNode));
    } else if (auto* arrayAssignNode = dynamic_cast<ArrayAssignNode*>(node)) {
        ir.push_back(generateArrayAssign(arrayAssignNode));
    } else if (auto* functionNode = dynamic_cast<FunctionNode*>(node)) {
        ir.push_back(generateFunction(functionNode));
    } else if (auto* literalNode = dynamic_cast<LiteralNode*>(node)) {
        ir.push_back(generateLiteral(literalNode));
    } else if (auto* functionCallNode = dynamic_cast<FunctionCallNode*>(node)) {
        ir.push_back(generateFunctionCall(functionCallNode));
    } else if (auto* builtInFunctionCallNode = dynamic_cast<BuiltInFunctionCallNode*>(node)) {
        ir.push_back(generateBuiltInFunctionCall(builtInFunctionCallNode));
    } else if (auto* serialFunctionsCallNode = dynamic_cast<SerialFunctionsCallNode*>(node)) {
        ir.push_back(generateSerialFunctionsCallNode(serialFunctionsCallNode));
    } else if (auto* methodCallNode = dynamic_cast<MethodCallNode*>(node)) {
        ir.push_back(generateMethodCall(methodCallNode));
    } else if (auto* varAssignNode = dynamic_cast<VarAssignNode*>(node)) {
        ir.push_back(generateVarAssign(varAssignNode));
    } else if (auto* unaryOpNode = dynamic_cast<UnaryOpNode*>(node)) {
        ir.push_back(generateUnaryOp(unaryOpNode));
    } else if (auto* binaryOpNode = dynamic_cast<BinaryOpNode*>(node)) {
        ir.push_back(generateBinaryOp(binaryOpNode));
    } else if (auto* compoundAssignNode = dynamic_cast<CompoundAssignNode*>(node)) {
        ir.push_back(generateCompoundAssign(compoundAssignNode));
    } else if (auto* ifNode = dynamic_cast<IfNode*>(node)) {
        ir.push_back(generateIfStatment(ifNode));
    } else if (auto* returnNode = dynamic_cast<ReturnNode*>(node)) {
        ir.push_back(generateReturnStatement(returnNode));
    } else if (auto* userFuncNode = dynamic_cast<UserFuncNode*>(node)) {
        ir.push_back(generateUserFunc(userFuncNode));
    } else if (auto* everyNode = dynamic_cast<EveryNode*>(node)) {
        ir.push_back(generateEveryStatement(everyNode));
    } else if (auto* whileNode = dynamic_cast<WhileNode*>(node)) {
        ir.push_back(generateWhileStatement(whileNode));
    } else if (auto* forRangeNode = dynamic_cast<ForRangeNode*>(node)) {
        ir.push_back(generateForRangeStatement(forRangeNode));
    } else if (auto* forNode = dynamic_cast<ForNode*>(node)) {
        ir.push_back(generateForStatement(forNode));
    } else if (auto* repeatNode = dynamic_cast<RepeatNode*>(node)) {
        ir.push_back(generateRepeatStatement(repeatNode));
    } else if (auto* groupNode = dynamic_cast<GroupNode*>(node)) {
        ir.push_back(generateGroupStatement(groupNode));
    } else if (auto* onPressNode = dynamic_cast<OnPressNode*>(node)) {
        ir.push_back(generateOnPressStatement(onPressNode));
    } else if (auto* controlTransferStatementsNode = dynamic_cast<ControlTransferStatementsNode*>(node)) {
        ir.push_back(generateControlTransferStatements(controlTransferStatementsNode));
    } else if (auto* typeConversionCallNode = dynamic_cast<TypeConversionCallNode*>(node)) {
        ir.push_back(generateTypeConversionCall(typeConversionCallNode));
    }

    return ir;
}