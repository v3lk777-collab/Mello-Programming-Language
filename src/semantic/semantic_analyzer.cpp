/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
*/

#include "semantic_analyzer.hpp"

DataType SemanticAnalyzer::mapStringToDataType(const std::string& typeStr) {
    if (typeStr == "int" || typeStr == "uint8_t" || typeStr == "int16_t" || typeStr == "uint16_t" || typeStr == "int32_t" || typeStr == "uint32_t") {
        return DataType::INT;
    }

    if (typeStr == "float") {
        return DataType::FLOAT;
    }

    if (typeStr == "String" || typeStr == "const char*") {
        return DataType::STRING;
    }

    if (typeStr == "bool") {
        return DataType::BOOL;
    }

    return DataType::UNKNOWN;
}

void SemanticAnalyzer::analyzeAssignment(VarAssignNode* varNode) {
    VariableSymbol symbol;

    symbol.name = varNode->name;
    symbol.type = mapStringToDataType(varNode->type);
    symbol.isConstant = varNode->isConstantVar;
    symbol.declaredLine = varNode->getCurrentDeclaredLine();

    std::string source = varNode->getSource();
    int currentLine = varNode->getCurrentDeclaredLine();
    int currentColumn = varNode->getCurrentDeclaredColumn();

    if (!symbolTable.declareVariable(symbol)) {
        ErrorHandler::report("This variable has already been declared:", symbol.name, currentLine, currentColumn, source);
    }
}

void SemanticAnalyzer::analyzeIfStatment(IfNode* ifNode) {
    if (ifNode->getCondition()) {
        analyzeNode(ifNode->getCondition().get());
    }

    symbolTable.enterScope();

    for (const auto& node : ifNode->getThenBody()) {
        analyzeNode(node.get());
    }

    for (const auto& node : ifNode->getElseBody()) {
        analyzeNode(node.get());
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeUserFuncDefinition(UserFuncNode* userFuncNode) {
    symbolTable.enterScope();

    std::string source = userFuncNode->getSource();
    int currentLine = userFuncNode->getCurrentDeclaredLine();
    int currentColumn = userFuncNode->getCurrentDeclaredColumn();

    for (const auto& paramName : userFuncNode->getFuncParams()) {
        VariableSymbol paramSymbol;

        paramSymbol.name = paramName;
        paramSymbol.type = DataType::UNKNOWN;
        paramSymbol.isConstant = false;
        paramSymbol.declaredLine = 0;

        if (!symbolTable.declareVariable(paramSymbol)) {
            ErrorHandler::report("Duplicate parameter name:", paramName, currentLine, currentColumn, source);
        }
    }

    for (const auto& node : userFuncNode->getFuncBody()) {
        analyzeNode(node.get());
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeWhileStatement(WhileNode* whileNode) {
    if (whileNode->getCondition()) {
        analyzeNode(whileNode->getCondition().get());
    }

    symbolTable.enterScope();

    for (const auto& body : whileNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeForStatement(ForNode* forNode) {
    if (forNode->getCondition()) {
        analyzeNode(forNode->getCondition().get());
    }

    symbolTable.enterScope();

    for (const auto& body : forNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeForRangeStatement(ForRangeNode* forRangeNode) {
    symbolTable.enterScope();

    for (const auto& body : forRangeNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeEveryStatement(EveryNode* everyNode) {
    symbolTable.enterScope();

    for (const auto& body : everyNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeUseStatement(UseNode* useNode) {
    std::string source = useNode->getSource();
    int currentLine = useNode->getCurrentDeclaredLine();
    int currentColumn = useNode->getCurrentDeclaredColumn();

    std::string libraryName = useNode->getLibraryName();

    if (!builtInLibraries.contains(libraryName)) {
        ErrorHandler::report("Unknown library:", libraryName, currentLine, currentColumn, source);
    }
}

void SemanticAnalyzer::analyzeRepeatStatement(RepeatNode* repeatNode) {
    symbolTable.enterScope();

    for (const auto& body : repeatNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeOnPressStatement(OnPressNode* onPressNode) {
    symbolTable.enterScope();

    for (const auto& body : onPressNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeFunctionDefinition(FunctionNode* funcNode) {
    symbolTable.enterScope();

    for (const auto& node : funcNode->getBody()) {
        analyzeNode(node.get());
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeLiteral(LiteralNode* literalNode) {
    if (literalNode->token.type == TokenType::IDENTIFIER) {
        const VariableSymbol* var = symbolTable.lookupVariable(literalNode->token.value);

        if (!var) {
            ErrorHandler::report("Use of undeclared variable:", literalNode->token.value, literalNode->token.line, literalNode->token.column, literalNode->token.value);
        }
    }
}

void SemanticAnalyzer::analyzeBinaryOp(BinaryOpNode* binaryOpNode) {
    if (binaryOpNode->getLeft()) {
        analyzeExpression(binaryOpNode->getLeft());
    }

    if (binaryOpNode->getRight()) {
        analyzeExpression(binaryOpNode->getRight());
    }
}

void SemanticAnalyzer::analyzeUnaryOp(UnaryOpNode* unaryOpNode) {
    if (unaryOpNode->getRight()) {
        analyzeExpression(unaryOpNode->getRight());
    }
}

void SemanticAnalyzer::analyzeGroup(GroupNode* groupNode) {
    if (groupNode->getExpression()) {
        analyzeExpression(groupNode->getExpression());
    }
}

void SemanticAnalyzer::analyzeFunctionCall(FunctionCallNode* functionCallNode) {
    std::string funcName = functionCallNode->getFuncName();
    const FunctionSymbol* func = symbolTable.lookupFunction(funcName);

    std::string source = functionCallNode->getSource();
    int currentLine = functionCallNode->getCurrentDeclaredLine();
    int currentColumn = functionCallNode->getCurrentDeclaredColumn();

    if (!func) {
        ErrorHandler::report("Call to undeclared function:", funcName, currentLine, currentColumn, source);
    }

    for (const auto& arg : functionCallNode->getArguments()) {
        analyzeExpression(arg.get());
    }
}

void SemanticAnalyzer::analyzeMethodCall(MethodCallNode* methodCallNode) {
    std::string objectName = methodCallNode->getObjectName();
    ASTNode* inner = methodCallNode->getMethodCall();

    if (!objectName.empty()) {
        if (objectName != "serial") {
            const VariableSymbol* objVar = symbolTable.lookupVariable(objectName);

            if (!objVar) {
                std::string source = methodCallNode->getSource();
                int currentLine = methodCallNode->getCurrentDeclaredLine();
                int currentColumn = methodCallNode->getCurrentDeclaredColumn();

                ErrorHandler::report("Use of undeclared object:", objectName, currentLine, currentColumn, source);

                return;
            }
        }

        if (auto functionCallNode = dynamic_cast<FunctionCallNode*>(inner)) {
            for (const auto& arg : functionCallNode->getArguments()) {
                analyzeExpression(arg.get());
            }

            return;
        }
    }

    if (auto serialCallNode = dynamic_cast<SerialFunctionsCallNode*>(inner)) {
        analyzeSerialCall(serialCallNode);
    } else if (auto builtInCallNode = dynamic_cast<BuiltInFunctionCallNode*>(inner)) {
        analyzeBuiltInCall(builtInCallNode);
    } else if (auto functionCallNode = dynamic_cast<FunctionCallNode*>(inner)) {
        analyzeFunctionCall(functionCallNode);
    }
}

void SemanticAnalyzer::analyzeSerialCall(SerialFunctionsCallNode* serialCallNode) {
    for (const auto& arg : serialCallNode->getArguments()) {
        analyzeExpression(arg.get());
    }
}

void SemanticAnalyzer::analyzeBuiltInCall(BuiltInFunctionCallNode* builtInCallNode) {
    for (const auto& arg : builtInCallNode->getArguments()) {
        analyzeExpression(arg.get());
    }
}

void SemanticAnalyzer::analyzeNode(ASTNode* node) {
    if (auto varNode = dynamic_cast<VarAssignNode*>(node)) {
        analyzeAssignment(varNode);
    } else if (auto funcNode = dynamic_cast<FunctionNode*>(node)) {
        analyzeFunctionDefinition(funcNode);
    } else if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        analyzeIfStatment(ifNode);
    } else if (auto userFuncNode = dynamic_cast<UserFuncNode*>(node)) {
        analyzeUserFuncDefinition(userFuncNode);
    } else if (auto whileNode = dynamic_cast<WhileNode*>(node)) {
        analyzeWhileStatement(whileNode);
    } else if (auto forNode = dynamic_cast<ForNode*>(node)) {
        analyzeForStatement(forNode);
    } else if (auto forRangeNode = dynamic_cast<ForRangeNode*>(node)) {
        analyzeForRangeStatement(forRangeNode);
    } else if (auto everyNode = dynamic_cast<EveryNode*>(node)) {
        analyzeEveryStatement(everyNode);
    } else if (auto useNode = dynamic_cast<UseNode*>(node)) {
        analyzeUseStatement(useNode);
    } else if (auto repeatNode = dynamic_cast<RepeatNode*>(node)) {
        analyzeRepeatStatement(repeatNode);
    } else if (auto onPressNode = dynamic_cast<OnPressNode*>(node)) {
        analyzeOnPressStatement(onPressNode);
    } else if (auto expressionrNode = dynamic_cast<ExpressionNode*>(node)) {
        analyzeExpression(expressionrNode);
    }
}

void SemanticAnalyzer::analyzeExpression(ExpressionNode* node) {
    if (auto literalNode = dynamic_cast<LiteralNode*>(node)) {
        analyzeLiteral(literalNode);
    } else if (auto binaryOpNode = dynamic_cast<BinaryOpNode*>(node)) {
        analyzeBinaryOp(binaryOpNode);
    } else if (auto unaryOpNode = dynamic_cast<UnaryOpNode*>(node)) {
        analyzeUnaryOp(unaryOpNode);
    } else if (auto groupNode = dynamic_cast<GroupNode*>(node)) {
        analyzeGroup(groupNode);
    } else if (auto functionCallNode = dynamic_cast<FunctionCallNode*>(node)) {
        analyzeFunctionCall(functionCallNode);
    } else if (auto methodCallNode = dynamic_cast<MethodCallNode*>(node)) {
        analyzeMethodCall(methodCallNode);
    } else if (auto serialCallNode = dynamic_cast<SerialFunctionsCallNode*>(node)) {
        analyzeSerialCall(serialCallNode);
    } else if (auto builtInCallNode = dynamic_cast<BuiltInFunctionCallNode*>(node)) {
        analyzeBuiltInCall(builtInCallNode);
    }
}