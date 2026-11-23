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

#include "avr_backend.hpp"

#include "ast.hpp"

#include <list>
#include <ranges>
#include <algorithm>

std::string AVRBackend::evaluateExpression(ExpressionNode* expression) {
    if (!expression) {
        return "";
    }

    std::stringstream tempStream;
    std::swap(output, tempStream);

    expression->accept(*this);

    std::swap(output, tempStream);

    return tempStream.str();
}

std::string AVRBackend::evaluateNode(ASTNode* node) {
    if (!node) {
        return "";
    }

    std::stringstream tempStream;

    std::swap(output, tempStream);

    node->accept(*this);

    std::swap(output, tempStream);

    return tempStream.str();
}

void AVRBackend::visit(ArrayNode* arrayNode) {
    const std::string& arrayName = arrayNode->getArrayName();
    const std::vector<std::string> arrayMembers = arrayNode->getArrayMembers() | std::views::transform([this] (const auto& expression) -> std::string { return evaluateExpression(expression.get()); }) | std::ranges::to<std::vector>();

    auto inferElementType = [&]() -> std::string {
        if (arrayMembers.empty()) {
            return "int";
        }

        const std::string& first = arrayMembers.front();

        if (!first.empty() && first.front() == '"') {
            return "const char*";
        } else if (first == "true" || first == "false") {
            return "bool";
        } else if (first.find('.') != std::string::npos) {
            return "float";
        } else {
            long long minVal = 0;
            long long maxVal = 0;
            bool allNumeric = true;

            for (const auto& member : arrayMembers) {
                try {
                    size_t pos = 0;
                    long long val = std::stoll(member, &pos, 0);

                    if (pos != member.size()) {
                        allNumeric = false;

                        break;
                    }

                    if (val < minVal) {
                        minVal = val;
                    }

                    if (val > maxVal) {
                        maxVal = val;
                    }
                } catch (...) {
                    allNumeric = false;

                    break;
                }
            }

            if (allNumeric) {
                if (minVal >= 0 && maxVal <= 255) {
                    return "uint8_t";
                } else if (minVal >= -32768 && maxVal <= 32767) {
                    return "int16_t";
                } else if (minVal >= 0 && maxVal <= 65535) {
                    return "uint16_t";
                } else {
                    return "int32_t";
                }
            }

            return "int";
        }
    };

    std::string type = inferElementType();

    output << type << " " << arrayName << "[" << arrayMembers.size() << "] = {";

    for (size_t i = 0; i < arrayMembers.size(); ++i) {
        output << arrayMembers[i];
        if (i + 1 < arrayMembers.size()) {
            output << ", ";
        }
    }

    output << "};\n";
}

void AVRBackend::visit(ArrayIndexNode* arrayIndexNode) {
    output << arrayIndexNode->getArrayName() << "[" << evaluateExpression(arrayIndexNode->getIndexExpression().get()) << "]";
}

void AVRBackend::visit(ArrayAssignNode* arrayAssignNode) {
    output << arrayAssignNode->getArrayName() << "[" << evaluateExpression(arrayAssignNode->getIndexExpression().get()) << "] = " << evaluateExpression(arrayAssignNode->getValueExpression().get()) << ";";
}

void AVRBackend::visit(ArrayBuiltInFunctionsNode* arrayBuiltInFunctionsNode) {
    const std::string& arrayName = arrayBuiltInFunctionsNode->getArrayName();
    const std::string& functionName = arrayBuiltInFunctionsNode->getFunctionName();
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments = arrayBuiltInFunctionsNode->getArguments();

    if (functionName == "len") {
        output << "(sizeof(" << arrayName << ") / sizeof(" << arrayName << "[0]))";
    }
}

void AVRBackend::visit(FunctionNode* functionNode) {
    std::string functionName = functionNode->getFunctionName();
    const std::vector<std::unique_ptr<ASTNode>>& body = functionNode->getBody();

    const int currentLine = functionNode->getCurrentDeclaredLine();
    const int currentColumn = functionNode->getCurrentDeclaredColumn();
    const std::string& source = functionNode->getSource();

    if (functionName == "start") {
        functionName = "setup";
    }

    output << "void " << functionName + "() {\n";

    if (functionName == "setup") {
        for (const auto& pin : inputPins) {
            output << "pinMode(" << pin << ", INPUT);\n";
        }

        for (const auto& pin : outputPins) {
            if (!inputPins.contains(pin)) {
                output << "pinMode(" << pin << ", OUTPUT);\n";
            } else {
                ErrorHandler::report("Error: This var was use in input pin", pin, currentLine, currentColumn, source);
            }
        }

        for (const auto& pin : inputPullUpPins) {
            if (!inputPins.contains(pin) && !outputPins.contains(pin)) {
                output << "pinMode(" << pin << ", INPUT_PULLUP);\n";
            } else {
                if (inputPins.contains(pin)) {
                    ErrorHandler::report("Error: This var was use in input pin", pin, currentLine, currentColumn, source);
                } else {
                    ErrorHandler::report("Error: This var was use in output pin", pin, currentLine, currentColumn, source);
                }
            }
        }

        if (!inputPins.empty() || !outputPins.empty() || !inputPullUpPins.empty()) {
            output << "\n";
        }
    }

    for (const auto& node : body) {
        node->accept(*this);
    }

    output << "}";
}

void AVRBackend::visit(LiteralNode* literalNode) {
    output << literalNode->getToken().value;
}

void AVRBackend::visit(FunctionCallNode* functionCallNode) {
    const std::string& functionName = functionCallNode->getFunctionName();
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments = functionCallNode->getArguments();
    bool isStatement = functionCallNode->getIsStatement();

    auto isNumber = [] (const std::string &str) {
        if (str.empty()) {
            return false;
        }

        for (const auto &c : str) {
            if (c == '-' || c == '+' || c == '*' || c == '/') {
                return !std::isdigit(c);
            }
        }

        return std::all_of(str.begin(), str.end(), [] (unsigned char c) {
            return std::isdigit(c);
        });
    };

    output << functionName << "(";

    for (size_t i = 0; i < arguments.size(); ++i) {
        arguments[i]->accept(*this);

        if (i < arguments.size() - 1) {
            output << ", ";
        }
    }

    output << ")" << (isStatement ? ";" : "");
}

void AVRBackend::visit(BuiltInFunctionCallNode* builtInFunctionCallNode) {
    const std::string& functionName = builtInFunctionCallNode->getFunctionName();
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments = builtInFunctionCallNode->getArguments();

    const int currentLine = builtInFunctionCallNode->getCurrentDeclaredLine();
    const int currentColumn = builtInFunctionCallNode->getCurrentDeclaredColumn();
    const std::string& source = builtInFunctionCallNode->getSource();

    if (!keywordsList.contains(functionName)) {
        ErrorHandler::report("'" + functionName + "' is not a known built-in command", functionName, currentLine, currentColumn, source);
    }

    std::vector<std::string> argsStr;

    argsStr.reserve(arguments.size());

    for (const auto& arg : arguments) {
        argsStr.push_back(evaluateExpression(arg.get()));
    }

    if (functionName == "turn_on" && argsStr.size() >= 1) {
        std::string pin = argsStr[0];

        output << "digitalWrite(" << pin << ", HIGH);";
    } else if (functionName == "turn_off" && argsStr.size() >= 1) {
        std::string pin = argsStr[0];

        output << "digitalWrite(" << pin << ", LOW);";
    } else if (functionName == "toggle" && argsStr.size() >= 1) {
        std::string pin = argsStr[0];
        std::string stateVar = "_toggle_state_" + pin;

        output << "static uint8_t " << stateVar << " = LOW;\n" << stateVar + " = !" << stateVar + ";\n" << "digitalWrite(" << pin + ", " << stateVar << ");";
    } else if (functionName == "write") {
        std::string pin = argsStr[0];
        std::string value = argsStr[1];

        if (variablesValues.contains(pin)) {
            std::string pinValue = variablesValues[pin];

            if (!(value == "0" || value == "1" || value == "HIGH" || value == "LOW")) {
                output << "analogWrite(" << pin << ", " << value << ");";

                return;
            }
        }

        if (value == "0" || value == "1" || value == "HIGH" || value == "LOW") {
            output << "digitalWrite(" << pin << ", " << value << ");";
        } else {
            output << "analogWrite(" << pin << ", " << value << ");";
        }
    } else if (functionName == "read") {
        std::string pin = argsStr[0];

        if (variablesValues.contains(pin)) {
            std::string pinValue = variablesValues[pin];

            if (pinValue.length() >= 2 && pinValue[0] == 'A' && isdigit(pinValue[1])) {
                output << "analogRead(" << pin << ")";

                return;
            }
        }

        if (pin.length() >= 2 && pin[0] == 'A' && isdigit(pin[1])) {
            output << "analogRead(" << pin << ")";
        } else {
            output << "digitalRead(" << pin << ")";
        }
    } else if (functionName == "wait" && argsStr.size() >= 1) {
        std::string fullArg = "";

        for (const auto& arg : argsStr) {
            fullArg += arg;
        }

        output << "delay(" << parseTime(fullArg, currentLine, currentColumn, source) << ");";
    } else if (functionName == "scale" && argsStr.size() >= 5) {
        output << "map(" << argsStr[0] << ", " << argsStr[1] << ", " << argsStr[2] << ", " << argsStr[3] << ", " << argsStr[4] << ")";
    } else if (functionName == "sleep") {
        if (!includedLibraries.contains("avr/sleep")) {
            includedLibraries.insert("avr/sleep");
        }

        std::string sleepModeType;
        std::string sleepMode = argsStr[0];

        if (sleepMode == "idle") {
            sleepModeType = "SLEEP_MODE_IDLE";
        } else if (sleepMode == "adc") {
            sleepModeType = "SLEEP_MODE_ADC";
        } else if (sleepMode == "deep") {
            sleepModeType = "SLEEP_MODE_PWR_DOWN";
        } else {
            ErrorHandler::report("Unknown sleep mode:", sleepMode, currentLine, currentColumn, source);
        }

        output << "set_sleep_mode(" << sleepModeType << ");\n";

        output << "sleep_enable();\n";
        output << "sleep_mode();\n";
        output << "sleep_disable();\n";
    } else if (functionName == "save_memory") {
        if (!includedLibraries.contains("EEPROM")) {
            includedLibraries.insert("EEPROM");
        }

        std::string address = argsStr[0];
        std::string value = argsStr[1];

        output << "EEPROM.put(" << address << ", " << value << ");\n";
    } else if (functionName == "read_memory") {
        if (!includedLibraries.contains("EEPROM")) {
            includedLibraries.insert("EEPROM");
        }

        std::string address = argsStr[0];

        output << "({ float val; EEPROM.get(" << address << ", val); val; })";
    } else if (functionName == "pulse_in") {
        if (argsStr.size() == 2) {
            std::string pin = argsStr[0];
            std::string value = argsStr[1];

            output << "pulseIn(" << pin << ", " << value << ")";

            return;
        }

        std::string timeInput = (argsStr.size() == 4) ? (argsStr[2] + argsStr[3]) : argsStr[2];
        std::string timeout = parseTime(timeInput, currentLine, currentColumn, source);
            
        std::string pin = argsStr[0];
        std::string value = argsStr[1];

        output << "pulseIn(" << pin << ", " << value << ", " << timeout << ")";
    } else if (functionName == "wait_ms") {
        output << "delayMicroseconds(" << argsStr[0] << ");";
    } else if (functionName == "pin_mode") {
        std::string pin = argsStr[0];
        std::string mode = argsStr[1];

        if (std::ranges::none_of(std::array{"INPUT", "OUTPUT", "INPUT_PULLUP"}, [&] (auto m) { return mode == m; })) {
            ErrorHandler::report("pin_mode() expects the second argument to be one of: INPUT, OUTPUT, INPUT_PULLUP", functionName, currentLine, currentColumn, source);
        }

        output << "pinMode(" << pin << ", " << mode << ");";
    }
}

void AVRBackend::visit(SerialFunctionsCallNode* serialFunctionsCallNode) {
    const std::string& functionName = serialFunctionsCallNode->getFunctionName();
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments = serialFunctionsCallNode->getArguments();

    const int currentLine = serialFunctionsCallNode->getCurrentDeclaredLine();
    const int currentColumn = serialFunctionsCallNode->getCurrentDeclaredColumn();
    const std::string& source = serialFunctionsCallNode->getSource();

    if (!serialKeywordsList.contains(functionName)) {
        ErrorHandler::report("'" + functionName + "' is not a known serial command", functionName, currentLine, currentColumn, source);
    }

    std::vector<std::string> argsStr;

    argsStr.reserve(arguments.size());

    for (const auto& arg : arguments) {
        argsStr.push_back(evaluateExpression(arg.get()));
    }

    if (functionName == "start" && argsStr.size() >= 1) {
        output << "Serial.begin(" << argsStr[0] << ");";
    } else if (functionName == "print" && argsStr.size() >= 1) {
        auto* literal = dynamic_cast<LiteralNode*>(arguments[0].get());
        bool isStringLiteral = literal && literal->getToken().type == TokenType::STRING;

        if (isStringLiteral) {
            output << "Serial.print(F(" << argsStr[0] << "));";
        } else {
            output << "Serial.print(" << argsStr[0] << ");";
        }
    } else if (functionName == "println" && argsStr.size() >= 1) {
        auto* literal = dynamic_cast<LiteralNode*>(arguments[0].get());
        bool isStringLiteral = literal && literal->getToken().type == TokenType::STRING;

        if (isStringLiteral) {
            output << "Serial.println(F(" << argsStr[0] << "));";
        } else {
            output << "Serial.println(" << argsStr[0] << ");";
        }
    } else if (functionName == "read") {
        output << "Serial.read();";
    } else if (functionName == "write" && argsStr.size() >= 1) {
        output << "Serial.write(" << argsStr[0] << ");";
    } else if (functionName == "available") {
        output << "Serial.available()";
    } else if (functionName == "availableForWrite") {
        output << "Serial.availableForWrite()";
    } else if (functionName == "end") {
        output << "Serial.end()";
    } else if (functionName == "readStringUntil" && argsStr.size() >= 1) {
        output << "Serial.readStringUntil(" << argsStr[0] << ");";
    } else if (functionName == "find" && argsStr.size() >= 2) {
        std::string target = argsStr[0];
        std::string length = argsStr[1];

        output << "Serial.find(" << target + ", " << length << ")";
    } else if (functionName == "findUntil" && argsStr.size() >= 2) {
        std::string target = argsStr[0];
        std::string terminal = argsStr[1];

        output << "Serial.findUntil(" << target + ", " << terminal << ")";
    } else if (functionName == "waitUntilSend") {
        output << "Serial.flush();";
    } else if (functionName == "parseFloat") {
        if (argsStr.size() >= 2) {
            output << "Serial.parseFloat(" << argsStr[0] << ", " << argsStr[1] << ");";
        } else if (argsStr.size() == 1) {
            output << "Serial.parseFloat(" << argsStr[0] << ");";
        } else {
            output << "Serial.parseFloat();";
        }
    } else if (functionName == "parseInt") {
        if (argsStr.size() >= 2) {
            output << "Serial.parseInt(" << argsStr[0] << ", " << argsStr[1] << ");";
        } else if (argsStr.size() == 1) {
            output << "Serial.parseInt(" << argsStr[0] << ");";
        } else {
            output << "Serial.parseInt();";
        }
    } else if (functionName == "peek") {
        output << "Serial.peek();";
    } else if (functionName == "millis") {
        output << "millis();";
    }

    output << "";
}

void AVRBackend::visit(MethodCallNode* methodCallNode) {
    const std::string& objectName = methodCallNode->getObjectName();
    ASTNode* methodCall = methodCallNode->getMethodCall();

    std::string methodStr = evaluateNode(methodCall);

    if (methodStr.starts_with("Serial.")) {
        methodStr = methodStr.substr(7);
    }

    if (!currentClassSelfName.empty() && objectName == currentClassSelfName) {
        output << "this->" << methodStr;

        return;
    }

    std::string finalObjectName = (objectName == "serial") ? "Serial" : objectName;
    bool isModuleName = userDefinedModulesNames.contains(finalObjectName);

    output << finalObjectName << (isModuleName ? "::" : ".") << methodStr;
}

void AVRBackend::visit(VarAssignNode* varAssignNode) {
    auto isNumeric = [] (const std::string& str) -> bool {
        if (str.empty()) {
            return false;
        }
        
        size_t start = 0;
        bool hasDecimal = false;
        
        if (str[0] == '-' || str[0] == '+') {
            if (str.size() == 1) {
                return false;
            }

            start = 1;
        }

        for (size_t i = start; i < str.size(); ++i) {
            if (str[i] == '.') {
                if (hasDecimal) {
                    return false;
                }
                
                hasDecimal = true;
            } else if (!std::isdigit(str[i])) {
                return false;
            }
        }

        return true;
    };

    auto inferIntegerType = [] (std::string value) -> std::string {
        std::string type;
        long long number = std::stoll(value);

        if (number >= 0 && number <= 255) {
            type = "uint8_t";
        } else if (number >= -32768 && number <= 32767) {
            type = "int16_t";
        } else if (number >= 0 && number <= 65535) {
            type = "uint16_t";
        } else if (number >= -2147483648LL && number <= 2147483647LL) {
            type = "int32_t";
        } else if (number >= 0 && number <= 4294967295LL) {
            type = "uint32_t";
        } else {
            type = "int32_t";
        }

        return type;
    };

    const std::string& name = varAssignNode->getName();
    ASTNode* valueNode = varAssignNode->getValue();
    std::string value = evaluateNode(varAssignNode->getValue());
    TokenType valueType = varAssignNode->getValueType();
    const std::string& rawValue = varAssignNode->getRawValue();

    bool isConstantVar = varAssignNode->getIsConstantVar();
    bool declaration = varAssignNode->isDeclaration();

    const int currentLine = varAssignNode->getCurrentDeclaredLine();
    const int currentColumn = varAssignNode->getCurrentDeclaredColumn();
    const std::string& source = varAssignNode->getSource();

    std::string type = "";

    if (!declaration) {
        bool wasString = stringVariables.contains(name);
        bool wasFloat = floatVariables.contains(name);
        bool wasInt = integerVariables.contains(name);

        bool isNowString = (valueType == TokenType::STRING) || value.find("\"") != std::string::npos;
        bool isNowFloat = !isNowString && value.find('.') != std::string::npos && isNumeric(value);
        bool isNowInt = !isNowString && !isNowFloat && isNumeric(value);

        bool mismatch = (wasString && !isNowString && (isNowInt || isNowFloat)) || ((wasFloat || wasInt) && isNowString);
        
        if (mismatch) {
            ErrorHandler::report("Type mismatch: cannot reassign variable:", name, currentLine, currentColumn, source);
        }

        output << name << " = " << value << ";";

        return;
    }

    auto containsWord = [&](const std::string expression, const std::string& word) -> bool {
        auto isIdentCharacter = [](char c) -> bool {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        };

        size_t position = expression.find(word);

        while (position != std::string::npos) {
            bool prevOk = (position == 0) || !isIdentCharacter(expression[position - 1]);

            size_t nextPosition = position + word.length();
            bool nextOk = (nextPosition == expression.length()) || !isIdentCharacter(expression[nextPosition]);

            if (prevOk && nextOk) {
                return true;
            }

            position = expression.find(word, position + 1);
        }

        return false;
    };

    auto containsNonConstantVariable = [&]() -> bool {
        for (const auto& var : nonConstantVariables) {
            if (containsWord(value, var)) {
                return true;
            }
        }

        for (const auto& param : currentFunctionParams) {
            if (containsWord(value, param)) {
                return true;
            }
        }

        return false;
    };

    auto containsStringVariable = [&](const std::string& expression) -> bool {
        for (const auto& stringVariable : stringVariables) {
            if (containsWord(expression, stringVariable)) {
                return true;
            }
        }

        return false;
    };

    auto containsFloatVariable = [&](const std::string& expression) -> bool {
        for (const auto& floatVariable : floatVariables) {
            if (containsWord(expression, floatVariable)) {
                return true;
            }
        }

        return false;
    };

    MethodCallNode* methodNode = dynamic_cast<MethodCallNode*>(valueNode);
    bool isModuleCall = methodNode && userDefinedModulesNames.contains(methodNode->getObjectName());

    SerialFunctionsCallNode* funcCall = dynamic_cast<SerialFunctionsCallNode*>(valueNode);

    bool isMethodOrMemberCall = (methodNode != nullptr) || (value.find("->") != std::string::npos) || (value.find('.') != std::string::npos);

    bool isFunctionCall = dynamic_cast<FunctionCallNode*>(valueNode) != nullptr || dynamic_cast<BuiltInFunctionCallNode*>(valueNode) != nullptr || dynamic_cast<MethodCallNode*>(valueNode) != nullptr || funcCall != nullptr || isModuleCall;

    if (!funcCall && methodNode) {
        funcCall = dynamic_cast<SerialFunctionsCallNode*>(methodNode->getMethodCall());
    }

    if (funcCall) {
        if (funcCall->getFunctionName() == "read") {
            type = "char";
            value = evaluateNode(funcCall);

            while (!value.empty() && value.back() == ';') {
                value.pop_back();
            }
        } else if (funcCall->getFunctionName() == "readStringUntil") {
            type = "String";
            value = evaluateNode(funcCall);

            while (!value.empty() && value.back() == ';') {
                value.pop_back();
            }
        } else if (funcCall->getVariableName() == "read_memory") {
            type = "float";

            floatVariables.insert(name);
        } else if (funcCall->getFunctionName() == "millis") {
            type = "long int";
        }
    } else if (value.find("atof(") != std::string::npos || value.find("(float)") != std::string::npos) {
        type = "float";
        floatVariables.insert(name);
    } else if (value.find("atoi(") != std::string::npos || value.find("(int)") != std::string::npos) {
        type = "int";
        integerVariables.insert(name);
    } else if (!isMethodOrMemberCall && valueType == TokenType::IDENTIFIER && value.find("(") != std::string::npos && value.back() == ')' && !userDefinedFunctionsNames.contains(value.substr(0, value.find("("))) && !userDefinedModulesNames.contains(value.substr(0, value.find("::")))) {
        std::string className = value.substr(0, value.find("("));

        if (!includedLibraries.contains(className)) {
            ErrorHandler::report("Must import this libarary first:", className, currentLine, currentColumn, source);
        }

        std::string args = value.substr(value.find("("));

        declaredVariables.insert(name);

        if (args == "()") {
            output << className << " " << name << ";\n";
        } else {
            output << className << " " << name << args << ";\n";
        }

        return;
    } else if (!isFunctionCall && (valueType == TokenType::STRING || value.find("\"") != std::string::npos || containsStringVariable(value) || value.find("String(") != std::string::npos)) {
        if (value.find("+") != std::string::npos) {
            type = "String";
            if (!value.starts_with("String(")) {
                value = "String(\"\") + " + value;
            }
        } else if (value.find("String(") != std::string::npos) {
            type = "String";
        } else {
            type = "const char*";
        }

        stringVariables.insert(name);
    } else if (valueType == TokenType::BOOLEAN && (value == "true" || value == "false")) {
        type = "bool";
    } else if (valueType == TokenType::FLOAT && (value.find('.') != std::string::npos || containsFloatVariable(value)) && isNumeric(value)) {
        type = "float";
        floatVariables.insert(name);
    } else if (valueType == TokenType::CHARACTER && value.length() == 1 && !isdigit(value[0])) {
        type = "char";
    } else if (valueType == TokenType::INTEGER && isNumeric(value)) {
        type = inferIntegerType(value);
        integerVariables.insert(name);
    } else {
        if (isModuleCall) {
            type = "auto";
        } else if (containsFloatVariable(value) || value.find("(float)") != std::string::npos || value.find("atof(") != std::string::npos) {
            type = "float";

            floatVariables.insert(name);
        } else if (value.find("(int)") != std::string::npos || value.find("atoi(") != std::string::npos) {
            type = "int";

            integerVariables.insert(name);
        } else {
            type = "int";

            integerVariables.insert(name);
        }
    }

    declaredVariables.insert(name);

    bool isReassigned = reassignedVariables.contains(name);
    bool hasConst = type.find("constexpr") != std::string::npos;
    bool isKeyword = keywordsList.contains(rawValue);
    bool isArrayAccess = value.find('[') != std::string::npos;

    FunctionCallNode* userFunction = dynamic_cast<FunctionCallNode*>(valueNode);

    if (!userFunction && isModuleCall) {
        userFunction = dynamic_cast<FunctionCallNode*>(methodNode->getMethodCall());
    }

    bool returnTypeKnown = false;

    if (userFunction) {
        if (auto it = userDefinedFunctionsReturnType.find(userFunction->getFunctionName()); it != userDefinedFunctionsReturnType.end()) {
            type = it->second;
            returnTypeKnown = true;

            integerVariables.erase(name);
            floatVariables.erase(name);
            stringVariables.erase(name);

            if (type == "float") {
                floatVariables.insert(name);
            } else if (type == "String" || type == "const char*") {
                stringVariables.insert(name);
            } else if (type.find("int") != std::string::npos) {
                integerVariables.insert(name);
            }
        }
    }

    if (isArrayAccess) {
        output << type << " " << name << " = " << value << ";\n";

        return;
    }

    if (((!isReassigned && !hasConst && !isKeyword) || isConstantVar) && !containsNonConstantVariable()) {
        if (type == "String") {
            if (isConstantVar) {
                output << "const " << type << " " << name << " = " << value << ";\n";
            } else {
                nonConstantVariables.insert(name);

                output << type << " " << name << " = " << value << ";\n";
            }
        } else if (isFunctionCall) {
            if (userFunction && !returnTypeKnown && !isMethodOrMemberCall) {
                ErrorHandler::report("Unknown function return type for:", userFunction->getFunctionName(), currentLine, currentColumn, source);
            }

            if (isMethodOrMemberCall) {
                type = "auto";
            }

            if (isConstantVar) {
                output << "const " << type << " " << name << " = " << value << ";\n";
            } else {
                nonConstantVariables.insert(name);

                output << type << " " << name << " = " << value << ";\n";
            }
        } else {
            if (userDefinedClassesNames.contains(currentScopeName)) {
                output << type << " " << name << " = " << value << ";\n";
            } else {
                output << "constexpr " << type << " " << name << " = " << value << ";\n";
            }
        }

        return;
    }

    nonConstantVariables.insert(name);

    output << type << " " << name << " = " << value << ";\n";
}

void AVRBackend::visit(UnaryOpNode* unaryOpNode) {
    const Token& op = unaryOpNode->getOp();
    std::string right = evaluateExpression(unaryOpNode->getRight());

    std::string opValue = op.value;

    if (opValue == "not") {
        opValue = "!";
    }

    output << opValue << right;
}

void AVRBackend::visit(BinaryOpNode* binaryOpNode) {
    const Token& op = binaryOpNode->getOp();
    std::string right = evaluateExpression(binaryOpNode->getRight());
    std::string left = evaluateExpression(binaryOpNode->getLeft());

    std::string opValue = op.value;

    if (opValue == "and") {
        opValue = "&&";
    } else if (opValue == "or") {
        opValue = "||";
    }

    if ((opValue == "==" || opValue == "!=")) {
        if (left == "Serial.read()" || left.find("command") != std::string::npos) {
            if (right.size() == 3 && right.front() == '"' && right.back() == '"') {
                right = "'" + right.substr(1, 1) + "'";
            }
        }
    }

    output << left << " " << opValue << " " << right;
}

void AVRBackend::visit(BitwiseNode* bitwiseNode) {
    const std::string& op = bitwiseNode->getOp();
    std::string right = evaluateExpression(bitwiseNode->getRight());
    std::string left = evaluateExpression(bitwiseNode->getLeft());

    output << left << " " << op << " " << right;
}

void AVRBackend::visit(CompoundAssignNode* compoundAssignNode) {
    const std::string& op = compoundAssignNode->getOp();
    const std::string& name = compoundAssignNode->getName();
    std::string value = evaluateExpression(compoundAssignNode->getValue());

    std::string target = name;

    if (!currentClassSelfName.empty()) {
        const std::string selfPrefix = currentClassSelfName + ".";

        if (target.compare(0, selfPrefix.length(), selfPrefix) == 0) {
            target = "this->" + target.substr(selfPrefix.length());
        }
    }

    if (op == "++") {
        output << target << "++;";

        return;
    }

    if (op == "--") {
        output << target << "--;";

        return;
    }

    output << target << " " << op + " " << value << ";";
}

void AVRBackend::visit(IfNode* ifNode) {
    const std::string& condition = evaluateNode(ifNode->getCondition().get());
    const std::vector<std::unique_ptr<ASTNode>>& thenBody = ifNode->getThenBody();
    const std::vector<std::unique_ptr<ASTNode>>& elseBody = ifNode->getElseBody();

    output << "if (" << condition << ") {\n";
        
    for (const auto& node : thenBody) {
        node->accept(*this);

        output << "\n";
    }

    output << "}";

    if (!elseBody.empty()) {
        if (elseBody.size() == 1 && dynamic_cast<IfNode*>(elseBody[0].get())) {
            std::string inner = evaluateNode(elseBody[0].get());
            output << "else " << inner;
        } else {
            output << "else {\n";
                
            for (const auto& node : elseBody) {
                node->accept(*this);

                output << "\n";
            }
                
            output << "}";
        }
    };
}

void AVRBackend::visit(ReturnNode* returnNode) {
    if (!returnNode->getValue()) {
        output << "return;";

        return;
    }

    std::string value = evaluateNode(returnNode->getValue());

    std::erase(value, ';');

    output << "return " << value << ";";
}

void AVRBackend::visit(UserFuncNode* userFuncNode) {
    const std::string& functionName = userFuncNode->getFunctionName();
    const std::string& userReturnType = userFuncNode->getUserReturnType();
    const std::vector<std::string>& functionParams = userFuncNode->getFuncParams();
    const std::vector<std::unique_ptr<ASTNode>>& body = userFuncNode->getBody();

    const int currentLine = userFuncNode->getCurrentDeclaredLine();
    const int currentColumn = userFuncNode->getCurrentDeclaredColumn();
    const std::string& source = userFuncNode->getSource();

    if (!functionParams.empty()) {
        output << "template<";

        for (size_t i = 0; i < functionParams.size(); i++) {
            output << "typename T" << std::to_string(i) << (i < functionParams.size() - 1 ? ", " : "");
        }

        output << ">\n";
    }

    currentFunctionParams = functionParams;

    const std::string returnType = (userReturnType == "void" || userReturnType == "any") ? "auto" : mapReturnTypeToString(userReturnType, functionName, currentLine, currentColumn, source);

    output << returnType << " " << functionName << "(";

    for (size_t i = 0; i < functionParams.size(); i++) {
        output << "T" << std::to_string(i) << " " << functionParams[i] << (i < functionParams.size() - 1 ? ", " : "");
    }

    output << ") {\n";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}\n";

    currentFunctionParams.clear();
}

void AVRBackend::visit(EveryNode* everyNode) {
    int id = everyNode->getID();
    const std::vector<std::unique_ptr<ASTNode>>& body = everyNode->getBody();

    const int currentLine = everyNode->getCurrentDeclaredLine();
    const int currentColumn = everyNode->getCurrentDeclaredColumn();
    const std::string& source = everyNode->getSource();

    std::string interval = parseTime(everyNode->getInterval(), currentLine, currentColumn, source);

    std::string timerName = "_every_timer_" + std::to_string(id);
    output << "static unsigned long " << timerName + " = 0;\n";

    output << "if (millis() - " << timerName << " >= " << interval << ") {\n";
    output << timerName << " = millis();\n";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}";
}

void AVRBackend::visit(WhileNode* whileNode) {
    const std::string& condition = evaluateExpression(whileNode->getCondition().get());
    const std::vector<std::unique_ptr<ASTNode>>& body = whileNode->getBody();

    output << "while (" << condition << ") {\n";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}\n";
}

void AVRBackend::visit(ForNode* forNode) {
    const std::string& condition = evaluateExpression(forNode->getCondition().get());
    const auto& body = forNode->getBody();

    std::string varName = forNode->getCondition()->getVariableName();

    bool isDecrement = condition.find(">") != std::string::npos;
    std::string op = isDecrement ? "--" : "++"; 

    output << "for (; " << condition << "; " << varName << op << ") {\n";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}\n";
}

void AVRBackend::visit(ForRangeNode* forRangeNode) {
    const std::string& varName = forRangeNode->getVarName();
    const std::string& start = evaluateExpression(forRangeNode->getStart().get());
    const std::string& stop = evaluateExpression(forRangeNode->getStop().get());
    const std::string& step = evaluateExpression(forRangeNode->getStep().get());
    const std::vector<std::unique_ptr<ASTNode>>& body = forRangeNode->getBody();

    output << "for (uint32_t " << varName << " = " << start << "; " << varName << " < " << stop << "; " << varName << " += " << step << ") {";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}\n";
}

void AVRBackend::visit(RepeatNode* repeatNode) {
    const int id = repeatNode->getID();
    const std::string& count = repeatNode->getCount();
    const std::vector<std::unique_ptr<ASTNode>>& body = repeatNode->getBody();

    std::string iterVar = "_loop_i_" + std::to_string(id);

    output << "for (int " << iterVar << " = 0; " << iterVar << " < " << count << "; " << iterVar << "++) {\n";
        
    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}\n";
}

void AVRBackend::visit(GroupNode* groupNode) {
    const std::string& expression = evaluateExpression(groupNode->getExpression());

    output << "(" << expression << ")";
}

void AVRBackend::visit(OnPressNode* onPressNode) {
    const int id = onPressNode->getID();
    const std::string& pin = onPressNode->getPin();
    const std::vector<std::unique_ptr<ASTNode>>& body = onPressNode->getBody();

    std::string timerVar = "_btn_timer_" + std::to_string(id);
    std::string lastVar = "_btn_last_" + std::to_string(id);
    std::string currVar = "_btn_curr_" + std::to_string(id);
    output << "static unsigned long " << timerVar << " = 0;\n";

    output << "static bool " << lastVar << " = HIGH;\n";
    output << "bool " << currVar << " = digitalRead(" << pin << ");\n";
    output << "if (" << currVar << " == LOW && " << lastVar << " == HIGH && (millis() - " << timerVar << " > 200)) {\n";
    output << timerVar << " = millis();\n";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}\n";
    output << lastVar << " = " << currVar << ";";
}

void AVRBackend::visit(ControlTransferStatementsNode* controlTransferStatementsNode) {
    const std::string& statement = controlTransferStatementsNode->getStatement();

    if (statement == "break") {
        output << "break;";
    } else if (statement == "continue") {
        output << "continue;";
    }
}

void AVRBackend::visit(TypeConversionCallNode* typeConversionCallNode) {
    const std::string& functionName = typeConversionCallNode->getFunctionName();
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments = typeConversionCallNode->getArguments();

    const int currentLine = typeConversionCallNode->getCurrentDeclaredLine();
    const int currentColumn = typeConversionCallNode->getCurrentDeclaredColumn();
    const std::string& source = typeConversionCallNode->getSource();

    if (arguments.empty()) {
        ErrorHandler::report("'" + functionName + "' requires exactly one argument", functionName, currentLine, currentColumn, source);
    }

    std::string value = evaluateExpression(arguments[0].get());

    auto* literal = dynamic_cast<LiteralNode*>(arguments[0].get());
    bool isStringArg = literal && literal->getToken().type == TokenType::STRING;

    if (functionName == "int") {
        if (isStringArg || stringVariables.contains(value)) {
            output << "atoi(" << value << ")";

            return;
        }

        output << "(int)(" << value << ")";

        return;
    }

    if (functionName == "float") {
        if (isStringArg || stringVariables.contains(value)) {
            output << "atof(" << value << ")";

            return;
        }

        output << "(float)(" << value << ")";

        return;
    }

    if (functionName == "str") {
        output << "String(" << value << ")";

        return;
    }

    ErrorHandler::report("'" + functionName + "' is not a known type conversion", functionName, currentLine, currentColumn, source);
}

void AVRBackend::visit(ModuleNode* moduleNode) {
    const std::string& moduleName = moduleNode->getModuleName();
    const std::vector<std::unique_ptr<ASTNode>>& body = moduleNode->getBody();

    output << "namespace " << moduleName << " {";

    for (const auto& node : body) {
        node->accept(*this);

        output << "\n";
    }

    output << "}";
}

void AVRBackend::visit(ClassNode* classNode) {
    const std::string& name = classNode->getClassName();
    const std::string& baseClass = classNode->getBaseClassName();
    const std::vector<std::unique_ptr<ASTNode>>& body = classNode->getBody();

    const int currentLine = classNode->getCurrentDeclaredLine();
    const int currentColumn = classNode->getCurrentDeclaredColumn();
    const std::string& source = classNode->getSource();

    const std::string previousScopeName = currentScopeName;

    currentScopeName = name;

    const std::string previousSelfName = currentClassSelfName;

    for (const auto& node : body) {
        auto functionNode = dynamic_cast<UserFuncNode*>(node.get());

        if (!functionNode) {
            continue;
        }

        const std::vector<std::string>& functionParams = functionNode->getFuncParams();

        if (functionParams.empty()) {
            ErrorHandler::report("Class method or constructor must have at least one parameter (e.g., 'self')", functionNode->getFunctionName(), currentLine, currentColumn, source);

            continue;
        }

        currentClassSelfName = functionParams[0];

        break;
    }

    bool hasBaseClass = !baseClass.empty();

    output << "class " << name << (hasBaseClass ? " : public " + baseClass + "" : " ") << "{\n";
    output << "public:\n\t";

    for (const auto& node : body) {
        if (auto functionNode = dynamic_cast<UserFuncNode*>(node.get())) {
            if (functionNode->getFunctionName() == "__init__") {
                const std::vector<std::string>& functionParams = functionNode->getFuncParams();

                if (functionParams.size() > 1) {
                    output << "template<";

                    for (size_t i = 1; i < functionParams.size(); ++i) {
                        output << "typename T" << std::to_string(i - 1) << (i < functionParams.size() - 1 ? ", " : "");
                    }

                    output << ">\n";
                }

                output << name << "(";

                for (size_t i = 1; i < functionParams.size(); ++i) {
                    output << "T" << std::to_string(i - 1) << " " << functionParams[i] << (i + 1 < functionParams.size() ? ", " : "");
                }

                output << ")" << (!functionNode->getBody().empty() ? " {\n" : " {}\n");

                if (!functionNode->getBody().empty()) {
                    for (const auto& node : functionNode->getBody()) {
                        node->accept(*this);

                        output << "\n";
                    }

                    output << "}\n";
                }

                continue;
            } else {
                if (std::ranges::contains(functionNode->params, currentClassSelfName)) {
                    std::erase(functionNode->params, currentClassSelfName);
                }
            }
        }

        node->accept(*this);

        output << "\n";
    }

    output << "};";

    currentScopeName = previousScopeName;
    currentClassSelfName = previousSelfName;
}

void AVRBackend::visit(ObjectCreationNode* objectCreationNode) {
    const std::string& object = objectCreationNode->getObjectName();

    const auto& arguments = objectCreationNode->getArguments();

    output << object << "(";

    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }

        output << evaluateExpression(arguments[i].get());
    }

    output << ")";
}

void AVRBackend::visit(MemberAccessNode* memberAccessNode) {
    const std::string& object = memberAccessNode->getObject();
    const std::string& memberName = memberAccessNode->getMemberName();

    if (object == currentClassSelfName) {
        output << "this->" << memberName;
    } else {
        output << object << "." << memberName;
    }
}

void AVRBackend::visit(MemberAssignNode* memberAssignNode) {
    const std::string& object = memberAssignNode->getObject();
    const std::string& memberName = memberAssignNode->getMemberName();
    const std::string& value = evaluateExpression(memberAssignNode->getValue());

    if (object == currentClassSelfName) {
        output << "this->" << memberName << " = " << value << ";\n";
    } else {
        output << object << "." << memberName << " = " << value << ";\n";
    }
}

std::string AVRBackend::generate(const std::vector<std::unique_ptr<ASTNode>>& stdLibProgram, const std::vector<std::unique_ptr<ASTNode>>& program) {
    bool hasSetup = false;
    bool hasLoop = false;
    bool hasSerial = false;
    bool hasSerialCommand = false;

    std::string includesCode;

    std::string stdLibGlobalVariablesCode;
    std::string stdLibFunctionsCode;

    std::string globalVariablesCode;
    std::string functionsCode;

    std::string author = "// Generated by Mello Programming Language\n"
                         "// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.\n"
                         "//\n"
                         "// This source code is private and protected by intellectual property laws\n"
                         "// Unauthorized use, modification, or distribution for any competitive\n"
                         "// academic or commercial purpose is strictly prohibited without\n"
                         "// explicit written permission from the author";

    for (const auto& node : stdLibProgram) {
        std::string code = evaluateNode(node.get());

        if (auto varNode = dynamic_cast<VarAssignNode*>(node.get())) {
            stdLibGlobalVariablesCode += code;
        } else {
            stdLibFunctionsCode += code + "\n";
        }
    }

    for (const auto& node : program) {
        std::string code = evaluateNode(node.get());

        if (dynamic_cast<VarAssignNode*>(node.get())) {
            globalVariablesCode += code;

            continue;
        }

        if (dynamic_cast<ClassNode*>(node.get())) {
            globalVariablesCode += code;

            continue;
        }

        if (code.find("void setup()") != std::string::npos) {
            hasSetup = true;
        }

        if (code.find("Serial.begin") != std::string::npos) {
            hasSerial = true;
        }

        if ((code.find("Serial.") != std::string::npos) && code.find("Serial.begin") == std::string::npos) {
            hasSerialCommand = true;
        }

        if (code.find("void loop()") != std::string::npos) {
            hasLoop = true;
        }

        functionsCode += code + "\n";
    }

    if (!includedLibraries.empty()) {
        for (const auto& lib : includedLibraries) {
            includesCode += "#include <" + lib + ".h>\n";
        }
    }

    std::stringstream outputFile;

    outputFile << author << "\n";
    outputFile << includesCode << "\n";

    outputFile << stdLibGlobalVariablesCode << "\n";
    outputFile << stdLibFunctionsCode << "\n";

    outputFile << globalVariablesCode << "\n";

    if (!hasSetup) {
        std::vector<std::unique_ptr<ASTNode>> setupBody;

        if (!hasSerial && hasSerialCommand) {
            std::vector<std::unique_ptr<ExpressionNode>> beginArgs;

            beginArgs.push_back(std::make_unique<LiteralNode>(Token(TokenType::INTEGER, "9600", 0, 0, "")));
            setupBody.push_back(std::make_unique<FunctionCallNode>("Serial.begin", std::move(beginArgs), true, 0, 0, ""));
        }

        FunctionNode autoSetup("setup", std::move(setupBody), 0, 0, "");

        outputFile << evaluateNode(&autoSetup) << "\n";
    } else if (!hasSerial && hasSerialCommand) {
        size_t pos = functionsCode.find("void setup() {");

        if (pos != std::string::npos) {
            size_t insertPos = functionsCode.find('\n', pos) + 1;

            functionsCode.insert(insertPos, "Serial.begin(9600);\n");
        }
    }

    outputFile << functionsCode << "\n";

    if (!hasLoop) {
        FunctionNode autoLoop("loop", {}, 0, 0, "");

        outputFile << evaluateNode(&autoLoop) << "\n";
    }

    return outputFile.str();
}