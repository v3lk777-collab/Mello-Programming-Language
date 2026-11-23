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

#include "semantic_analyzer.hpp"

#include "utils.hpp"

void SemanticAnalyzer::analyzeAssignment(VarAssignNode* varNode) {
    if (varNode->getValue()) {
        analyzeNode(varNode->getValue());
    }

    if (auto* objCreation = dynamic_cast<ObjectCreationNode*>(varNode->getValue())) {
        const std::string& className = objCreation->getObjectName();
        const std::string& instanceName = varNode->name;

        if (classOutputMembers.contains(className)) {
            for (const auto& member : classOutputMembers[className]) {
                outputPins.insert(instanceName + "." + member);
            }
        }

        if (classInputMembers.contains(className)) {
            for (const auto& member : classInputMembers[className]) {
                inputPins.insert(instanceName + "." + member);
            }
        }

        if (classInputPullUpMembers.contains(className)) {
            for (const auto& member : classInputPullUpMembers[className]) {
                inputPullUpPins.insert(instanceName + "." + member);
            }
        }
    }

    if (symbolTable.lookupVariable(varNode->name)) {
        varNode->setIsDeclaration(false);

        return;
    }

    varNode->setIsDeclaration(true);

    VariableSymbol symbol;

    symbol.name = varNode->name;
    symbol.type = mapTokenTypeToDataType(varNode->val_type);
    symbol.isConstant = varNode->isConstantVar;
    symbol.declaredLine = varNode->getCurrentDeclaredLine();

    symbolTable.declareVariable(symbol);
}

void SemanticAnalyzer::analyzeArrayDefinition(ArrayNode* arrayNode) {
    VariableSymbol array;

    array.name = arrayNode->getArrayName();
    array.type = DataType::ARRAY;
    array.declaredLine = arrayNode->getCurrentDeclaredLine();

    if (!symbolTable.declareVariable(array)) {
        ErrorHandler::report("This array has already been declared:", array.name, arrayNode->getCurrentDeclaredLine(), arrayNode->getCurrentDeclaredColumn(), arrayNode->getSource());
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
    DataType userReturnType = mapStringToDataType(userFuncNode->getUserReturnType());

    FunctionSymbol function;

    function.name = currentScopeName.empty() ? userFuncNode->getFunctionName() : (currentScopeName + "::" + userFuncNode->getFunctionName());

    function.paramNames = userFuncNode->getFuncParams();
    function.returnType = userReturnType;

    std::string source = userFuncNode->getSource();
    int currentLine = userFuncNode->getCurrentDeclaredLine();
    int currentColumn = userFuncNode->getCurrentDeclaredColumn();

    if (!symbolTable.declareFunction(function)) {
        ErrorHandler::report("This function has already been declared:", function.name, currentLine, currentColumn, source);
    }

    symbolTable.enterScope();

    for (const auto& paramName : userFuncNode->getFuncParams()) {
        VariableSymbol paramSymbol;

        paramSymbol.name = paramName;
        paramSymbol.type = DataType::ANY;
        paramSymbol.isConstant = false;
        paramSymbol.declaredLine = 0;

        if (!symbolTable.declareVariable(paramSymbol)) {
            ErrorHandler::report("Duplicate parameter name:", paramName, currentLine, currentColumn, source);
        }
    }

    bool hasReturn = false;

    auto isTypeCompatible = [] (DataType expected, DataType actual) -> bool {
        if (expected == DataType::ANY || actual == DataType::ANY) {
            return true;
        }

        if (expected == actual) {
            return true;
        }

        if (expected == DataType::FLOAT && actual == DataType::INTEGER) {
            return true;
        }

        return false;
    };

    auto validateReturns = [&] (this auto&& self, const std::unique_ptr<ASTNode>& node) -> void {
        if (!node) {
            return;
        }

        if (auto* returnNode = dynamic_cast<ReturnNode*>(node.get())) {
            hasReturn = true;

            if (auto literalNode = dynamic_cast<LiteralNode*>(returnNode->getValue())) {
                DataType returnType = DataType::ANY;

                if (literalNode->getToken().type == TokenType::IDENTIFIER) {
                    if (auto* variableSymbol = symbolTable.lookupVariable(literalNode->getToken().value)) {
                        returnType = variableSymbol->type;
                    }
                } else {
                    returnType = mapTokenTypeToDataType(literalNode->getToken().type);
                }

                if (userReturnType != DataType::ANY && !isTypeCompatible(userReturnType, returnType)) {
                    ErrorHandler::report("Function return type doesn't match the return type", function.name, currentLine, currentColumn, source);
                }
            }
        }

        for (const auto* child : node->getChildBodies()) {
            for (const auto& node : *child) {
                self(node);
            }
        }
    };

    for (const auto& node : userFuncNode->getBody()) {
        analyzeNode(node.get());

        validateReturns(node);
    }

    if (!hasReturn && userReturnType != DataType::ANY && userReturnType != DataType::VOID) {
        ErrorHandler::report("Function declares a return type but has no return statement:", function.name, currentLine, currentColumn, source);
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

    if (!forRangeNode->getVarName().empty()) {
        VariableSymbol variable;

        variable.name = forRangeNode->getVarName();

        symbolTable.declareVariable(variable);
    }

    if (forRangeNode->getStart()) {
        analyzeExpression(forRangeNode->getStart().get());
    }

    if (forRangeNode->getStop()) {
        analyzeExpression(forRangeNode->getStop().get());
    }

    if (forRangeNode->getStep()) {
        analyzeExpression(forRangeNode->getStep().get());
    }

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
    std::string buttonPin = onPressNode->getPin();

    if (!buttonPin.empty()) {
        inputPullUpPins.insert(buttonPin);
        classInputPullUpMembers[currentScopeName].insert(buttonPin);
    }

    symbolTable.enterScope();

    for (const auto& body : onPressNode->getChildBodies()) {
        for (const auto& node : *body) {
            analyzeNode(node.get());
        }
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeFunctionDefinition(FunctionNode* funcNode) {
    FunctionSymbol function;

    function.name = funcNode->getFunctionName();

    std::string source = funcNode->getSource();
    int currentLine = funcNode->getCurrentDeclaredLine();
    int currentColumn = funcNode->getCurrentDeclaredColumn();

    if (!symbolTable.declareFunction(function)) {
        ErrorHandler::report("This function has already been declared:", function.name, currentLine, currentColumn, source);
    }

    symbolTable.enterScope();

    for (const auto& node : funcNode->getBody()) {
        analyzeNode(node.get());
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzeLiteral(LiteralNode* literalNode) {
    const Token& token = literalNode->getToken();

    if (token.type == TokenType::IDENTIFIER) {
        const VariableSymbol* var = symbolTable.lookupVariable(token.value);

        if (var && (var->name.starts_with("A") && std::all_of(var->name.begin() + 1, var->name.end(), [] (char c) { return isdigit(c); }))) {
            ErrorHandler::report("Use of undeclared variable:", token.value, token.line, token.column, token.value);
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
    std::string funcName = functionCallNode->getFunctionName();

    if (userDefinedClassesNames.contains(funcName)) {
        for (const auto& arg : functionCallNode->getArguments()) {
            analyzeExpression(arg.get());
        }

        return;
    }

    const FunctionSymbol* func = nullptr;

    if (!currentScopeName.empty()) {
        func = symbolTable.lookupFunction(currentScopeName + "::" + funcName);
    }

    if (!func) {
        func = symbolTable.lookupFunction(funcName);
    }

    std::string source = functionCallNode->getSource();
    int currentLine = functionCallNode->getCurrentDeclaredLine();
    int currentColumn = functionCallNode->getCurrentDeclaredColumn();

    if (!func) {
        ErrorHandler::report("Call to undeclared function:", funcName, currentLine, currentColumn, source);
    }

    int userFunctionParamsNumber = 0;

    if (auto it = userDefinedFunctionsParamsNumber.find(functionCallNode->getFunctionName()); it != userDefinedFunctionsParamsNumber.end()) {
        userFunctionParamsNumber = it->second;
    }

    int functionParamsNumber = functionCallNode->getFuncArgumentsSize();

    if ((functionParamsNumber > userFunctionParamsNumber) || (functionParamsNumber < userFunctionParamsNumber)) {
        ErrorHandler::report(std::format("This function takes {} argument, but it was taken {} argument:", userFunctionParamsNumber, functionParamsNumber), functionCallNode->getFunctionName(), currentLine, currentColumn, source);
    }

    for (const auto& arg : functionCallNode->getArguments()) {
        analyzeExpression(arg.get());
    }
}

void SemanticAnalyzer::analyzeMethodCall(MethodCallNode* methodCallNode) {
    std::string objectName = methodCallNode->getObjectName();
    ASTNode* inner = methodCallNode->getMethodCall();

    if (!objectName.empty()) {
        if (objectName != "serial" && !arraysNamesList.contains(objectName) && !stdLibs.contains(objectName) && !arduinoLibraries.contains(objectName) && !userDefinedModulesNames.contains(objectName) && !userIncludedFiles.contains(objectName)) {
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
    const std::string& funcName = builtInCallNode->getFunctionName();
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments = builtInCallNode->getArguments();

    std::string source = builtInCallNode->getSource();
    int currentLine = builtInCallNode->getCurrentDeclaredLine();
    int currentColumn = builtInCallNode->getCurrentDeclaredColumn();

    auto getPinNameFromExpression = [] (ExpressionNode* expression) -> std::string {
        if (!expression) {
            return "";
        }

        if (auto* it = dynamic_cast<LiteralNode*>(expression)) {
            return it->getToken().value;
        } else if (auto* it = dynamic_cast<MemberAccessNode*>(expression)) {
            return it->getMemberName();
        }

        return "";
    };

    auto it = builtInFunctions.find(funcName);

    if (it == builtInFunctions.end()) {
        ErrorHandler::report("Unknown function called " + funcName, "", currentLine, currentColumn, source);

        return;
    }

    if (arguments.size() != it->second) {
        ErrorHandler::report(std::format("{}() expects {} arguments, got {}", funcName, it->second, arguments.size()), "", currentLine, currentColumn, source);

        return;
    }

    for (const auto& argument : arguments) {
        analyzeExpression(argument.get());
    }

    auto resolvePin = [&](const std::string& name) -> std::string {
        if (name.empty() || currentScopeName.empty() || funcParams.contains(name)) {
            return name;
        }

        bool isNumber = std::isdigit(static_cast<unsigned char>(name[0]));
        bool isAnalog = (name.size() >= 2 && name[0] == 'A' && std::isdigit(static_cast<unsigned char>(name[1])));

        if (!isNumber && !isAnalog) {
            return currentScopeName + (userDefinedModulesNames.contains(currentScopeName) ? "::" : ".") + name;
        }

        return name;
    };

    if (!arguments.empty()) {
        std::string pinName = getPinNameFromExpression(arguments[0].get());
            
        if (!currentParsingUserFunc.empty() && funcParams.contains(pinName)) {
            auto it = std::find(currentFuncParamNames.begin(), currentFuncParamNames.end(), pinName);

            if (it != currentFuncParamNames.end()) {
                size_t idx = std::distance(currentFuncParamNames.begin(), it);

                if (funcName == "read" || funcName == "pulse_in") {
                    userFuncInputParams[currentParsingUserFunc][idx] = true;
                } else if (funcName == "turn_on" || funcName == "turn_off" || funcName == "toggle" || funcName == "write") {
                    userFuncOutputParams[currentParsingUserFunc][idx] = true;
                } else {
                    if (userFuncOutputParams.contains(funcName)) {
                        for (size_t i = 0; i < arguments.size() && i < userFuncOutputParams[funcName].size(); ++i) {
                            if (userFuncOutputParams[funcName][i]) {
                                std::string argPin = getPinNameFromExpression(arguments[i].get());
                                auto pit = std::find(currentFuncParamNames.begin(), currentFuncParamNames.end(), argPin);

                                if (pit != currentFuncParamNames.end()) {
                                    size_t pidx = std::distance(currentFuncParamNames.begin(), pit);

                                    userFuncOutputParams[currentParsingUserFunc][pidx] = true;
                                }
                            }
                        }
                    }
                }
            }
        } else if (userDefinedClassesNames.contains(currentScopeName)) {
            if (funcName == "read" || funcName == "pulse_in") {
                classInputMembers[currentScopeName].insert(pinName);
            } else if (funcName == "turn_on" || funcName == "turn_off" || funcName == "toggle" || funcName == "write") {
                classOutputMembers[currentScopeName].insert(pinName);
            }
        } else if (funcName == "read" || funcName == "pulse_in") {
            inputPins.insert(resolvePin(pinName));
        } else if (funcName == "turn_on" || funcName == "turn_off" || funcName == "toggle" || funcName == "write") {
            outputPins.insert(resolvePin(pinName));
        } else {
            if (userFuncOutputParams.contains(funcName)) {
                for (size_t i = 0; i < arguments.size() && i < userFuncOutputParams[funcName].size(); ++i) {
                    if (userFuncOutputParams[funcName][i]) {
                        outputPins.insert(getPinNameFromExpression(arguments[i].get()));
                    }
                }
            }

            if (userFuncInputParams.contains(funcName)) {
                for (size_t i = 0; i < arguments.size() && i < userFuncInputParams[funcName].size(); ++i) {
                    if (userFuncInputParams[funcName][i]) {
                        inputPins.insert(getPinNameFromExpression(arguments[i].get()));
                    }
                }
            }
        }
    }
}

void SemanticAnalyzer::analyzeReturnStatement(ReturnNode* returnNode) {
    if (returnNode->getValue()) {
        analyzeNode(returnNode->getValue());
    }
}

void SemanticAnalyzer::analyzeModule(ModuleNode* moduleNode) {
    const std::string previousScope = currentScopeName;

    currentScopeName = moduleNode->getModuleName();

    symbolTable.enterScope();

    for (const auto& node : moduleNode->getBody()) {
        analyzeNode(node.get());
    }

    symbolTable.exitScope();

    currentScopeName = previousScope;
}

void SemanticAnalyzer::analyzeClass(ClassNode* classNode) {
    const std::string previousScope = currentScopeName;

    currentScopeName = classNode->getClassName();

    userDefinedClassesNames.insert(currentScopeName);

    symbolTable.enterScope();

    for (const auto& node : classNode->getBody()) {
        if (auto* functionNode = dynamic_cast<UserFuncNode*>(node.get()); functionNode && functionNode->getFunctionName() == "__init__") {
            continue;
        }

        analyzeNode(node.get());
    }

    symbolTable.exitScope();

    currentScopeName = previousScope;
}

void SemanticAnalyzer::analyzeObjectCreationNode(ObjectCreationNode* objectCreationNode) {
    const std::string previousScope = currentScopeName;

    currentScopeName = objectCreationNode->getObjectName();

    for (const auto& argument : objectCreationNode->getArguments()) {
        analyzeExpression(argument.get());
    }

    currentScopeName = previousScope;
}

void SemanticAnalyzer::analyzeNode(ASTNode* node) {
    if (auto varNode = dynamic_cast<VarAssignNode*>(node)) {
        analyzeAssignment(varNode);
    } else if (auto funcNode = dynamic_cast<FunctionNode*>(node)) {
        analyzeFunctionDefinition(funcNode);
    } else if (auto arrayNode = dynamic_cast<ArrayNode*>(node)) {
        analyzeArrayDefinition(arrayNode);
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
    } else if (auto moduleNode = dynamic_cast<ModuleNode*>(node)) {
        analyzeModule(moduleNode);
    } else if (auto classNode = dynamic_cast<ClassNode*>(node)) {
        analyzeClass(classNode);
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
    } else if (auto returnNode = dynamic_cast<ReturnNode*>(node)) {
        analyzeReturnStatement(returnNode);
    } else if (auto objectCreationNode = dynamic_cast<ObjectCreationNode*>(node)) {
        analyzeObjectCreationNode(objectCreationNode);
    }
}