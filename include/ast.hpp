// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once

#include "utils.hpp"
#include "lexer.hpp"

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toCpp() = 0;
};

class ExpressionNode : public ASTNode {
public:
    virtual ~ExpressionNode() = default;
    virtual std::string getVariableName() {
        return "";
    };
};

class VarAssignNode : public ASTNode {
public:
    std::string name;
    bool isConstantVar;
    TokenType val_type;
    std::string raw_value;
    std::unique_ptr<ASTNode> value;

private:
    bool isNumeric(const std::string& str) const noexcept {
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
    }

    std::string inferIntegerType(std::string value) const {
        std::string type;
        long long number = std::stoll(value);

        if (number >= 0 && number <= 255) {
            type = "uint8_t";
        } else if (number >= -32768 && number <= 32768) {
            type = "int";
        } else if (number >= 0 && number <= 65535) {
            type = "uint16_t";
        } else if (number >= -2147483648LL && number <= 2147483647LL) {
            type = "long";
        } else if (number >= 0 && number <= 4294967295LL) {
            type = "unsigned long";
        } else {
            type = "int";
        }

        return type;
    }

public:
    VarAssignNode(std::string name, std::unique_ptr<ASTNode> value, std::string raw, TokenType val_type, bool isConstantVar) 
        : name(std::move(name)), value(std::move(value)), raw_value(std::move(raw)), val_type(val_type), isConstantVar(isConstantVar) {}

public:
    std::string toCpp() override {
        std::string type = "";
        std::string final_value = value->toCpp();

        while (!final_value.empty() && (final_value.back() == ';' || final_value.back() == '\n' || final_value.back() == ' ')) {
            final_value.pop_back();
        }

        if (declaredVariables.count(name)) {
            return name + " = " + final_value + ";";
        }

        auto containsStringVariable = [&](const std::string& expr) {
            for (const auto& stringVariable : stringVariables) {
                if (expr.find(stringVariable) != std::string::npos) {
                    return true;
                }
            }

            return false;
        };

        auto containsFloatVariable = [&](const std::string& expr) {
            for (const auto& floatVariable : floatVariables) {
                if (expr.find(floatVariable) != std::string::npos) {
                    return true;
                }
            }

            return false;
        };

        if (final_value.find("Serial.read()") != std::string::npos) {
            type = "char";
            final_value = "Serial.read()";
        } else if (val_type == TokenType::SYMBOL && final_value.find("(") != std::string::npos && final_value.back() == ')') {
            std::string className = final_value.substr(0, final_value.find("("));
            std::string args = final_value.substr(final_value.find("("));

            declaredVariables.insert(name);
            includedLibraries.insert(className);

            if (args == "()") {
                return className + " " + name + ";\n";
            } else {
                return className + " " + name + args + ";\n";
            }
        } else if (val_type == TokenType::STRING || final_value.find("\"") != std::string::npos || containsStringVariable(final_value)) {
            if (final_value.find("+") != std::string::npos) {
                type = "String";
                final_value = "String(\"\") + " + final_value; 
            } else {
                type = "const char*";
            }

            stringVariables.insert(name);
        } else if (final_value == "true" || final_value == "false") {
            type = "bool";
        } else if ((final_value.find('.') != std::string::npos || containsFloatVariable(final_value)) && isNumeric(final_value)) {
            type = "float";
            floatVariables.insert(name);
        } else if (final_value.length() == 1 && !isdigit(final_value[0])) {
            type = "char";
            final_value = "'" + final_value + "'";
        } else if (isNumeric(final_value)) {
            type = inferIntegerType(final_value);
            integerVariables.insert(name);
        } else {
            if (containsFloatVariable(final_value)) {
                type = "float";
                floatVariables.insert(name);
            } else {
                type = "int";
                integerVariables.insert(name);
            }
        }
        
        declaredVariables.insert(name);

        bool isReassigned = reassignedVariables.count(name) > 0;
        bool hasConst = type.find("constexpr") != std::string::npos;
        bool isKeyword = keywordsList.count(raw_value) > 0;

        if ((!isReassigned && !hasConst && !isKeyword) || isConstantVar) {
            return "constexpr " + type + " " + name + " = " + final_value + ";\n";
        }

        return type + " " + name + " = " + final_value + ";\n";
    }
};

class ArrayNode : public ASTNode {
private:
    std::string arrayName;
    std::vector<std::string> arrayMembers;

private:
    std::string inferElementType() const {
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
            return "int";
        }
    }

public:
    ArrayNode(std::string name, std::vector<std::string> arrayMembers)
        : arrayName(std::move(name)), arrayMembers(std::move(arrayMembers)) {}

public:
    std::string toCpp() override {
        std::string type = inferElementType();
        std::string result = type + " " + arrayName + "[" + std::to_string(arrayMembers.size()) + "] = {";

        for (size_t i = 0; i < arrayMembers.size(); ++i) {
            result += arrayMembers[i];
            if (i + 1 < arrayMembers.size()) result += ", ";
        }

        result += "};\n";

        return result;
    }
};

class ArrayIndexNode : public ExpressionNode {
private:
    std::string arrayName;
    std::unique_ptr<ExpressionNode> indexExpression;

public:
    ArrayIndexNode(std::string arrayName, std::unique_ptr<ExpressionNode> indexExpression)
        : arrayName(std::move(arrayName)), indexExpression(std::move(indexExpression)) {}

public:
    std::string toCpp() override {
        return arrayName + "[" + indexExpression->toCpp() + "]";
    }
};

class FunctionNode : public ASTNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    FunctionNode(std::string name, std::vector<std::unique_ptr<ASTNode>> funcBody)
        : funcName(std::move(name)), body(std::move(funcBody)) {}

public:
    std::string toCpp() override {
        if (funcName == "start") {
            funcName = "setup";
        }

        std::string result = "void " + funcName + "() {\n";
        
        if (funcName == "setup") {
            for (const auto& pin : inputPins) {
                result += "pinMode(" + pin + ", INPUT);\n";
            }

            for (const auto& pin : outputPins) {
                if (!inputPins.count(pin)) {
                    result += "pinMode(" + pin + ", OUTPUT);\n";
                }
            }

            if (!inputPins.empty() || !outputPins.empty()) {
                result += "\n";
            }
        }

        for (const auto& node : body) {
            result += node->toCpp() + "\n";
        }

        result += "}";
        return result;
    }
};

class LiteralNode : public ExpressionNode {
public:
    Token token;

public:
    LiteralNode(Token t)
        : token(t) {}

    std::string toCpp() override {
        return token.value;
    }
};

class FunctionCallNode : public ExpressionNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

private:
    bool isNumber(const std::string &str) {
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
    }

public:
    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> args)
        : funcName(std::move(name)), arguments(std::move(args)) {
        
        if (!arguments.empty()) {
            std::string pinName = arguments[0]->toCpp();
            
            if (!currentParsingUserFunc.empty() && funcParams.count(pinName)) {
                auto it = std::find(currentFuncParamNames.begin(), currentFuncParamNames.end(), pinName);
                if (it != currentFuncParamNames.end()) {
                    size_t idx = std::distance(currentFuncParamNames.begin(), it);
                    if (funcName == "read") {
                        userFuncInputParams[currentParsingUserFunc][idx] = true;
                    } else if (funcName == "turn_on" || funcName == "turn_off" || funcName == "toggle" || (funcName == "write" && arguments.size() >= 2)) {
                        userFuncOutputParams[currentParsingUserFunc][idx] = true;
                    } else {
                        if (userFuncOutputParams.count(funcName)) {
                            for (size_t i = 0; i < arguments.size() && i < userFuncOutputParams[funcName].size(); ++i) {
                                if (userFuncOutputParams[funcName][i]) {
                                    std::string argPin = arguments[i]->toCpp();
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
            } else if (funcName == "read") {
                inputPins.insert(pinName);
            } else if (funcName == "turn_on" || funcName == "turn_off" || funcName == "toggle" || (funcName == "write" && arguments.size() >= 2)) {
                outputPins.insert(pinName);
            } else {
                if (userFuncOutputParams.count(funcName)) {
                    for (size_t i = 0; i < arguments.size() && i < userFuncOutputParams[funcName].size(); ++i) {
                        if (userFuncOutputParams[funcName][i]) {
                            outputPins.insert(arguments[i]->toCpp());
                        }
                    }
                }

                if (userFuncInputParams.count(funcName)) {
                    for (size_t i = 0; i < arguments.size() && i < userFuncInputParams[funcName].size(); ++i) {
                        if (userFuncInputParams[funcName][i]) {
                            inputPins.insert(arguments[i]->toCpp());
                        }
                    }
                }
            }
        }
    }

public:
    std::string toCpp() override {
        std::vector<std::string> argsStr;

        for (const auto& arg : arguments) {
            argsStr.push_back(arg->toCpp());
        }

        if (funcName == "turn_on" && argsStr.size() >= 1) {
            std::string pin = argsStr[0];
            return "digitalWrite(" + pin + ", HIGH);";
        }

        if (funcName == "turn_off" && argsStr.size() >= 1) {
            std::string pin = argsStr[0];
            return "digitalWrite(" + pin + ", LOW);";
        }

        if (funcName == "toggle" && argsStr.size() >= 1) {
            std::string pin = argsStr[0];
            std::string stateVar = "_toggle_state_" + pin;

            return "static uint8_t " + stateVar + " = LOW;\n" + stateVar + " = (" + stateVar + " == LOW) ? HIGH : LOW;\n" + "digitalWrite(" + pin + ", " + stateVar + ");";
        }

        if (funcName == "write") {
            if (argsStr.size() >= 2) {
                std::string pin = argsStr[0];
                std::string value = argsStr[1];

                if (value == "0" || value == "1" || value == "HIGH" || value == "LOW") {
                    return "digitalWrite(" + pin + ", " + value + ");";
                } else {
                    return "analogWrite(" + pin + ", " + value + ");";
                }
            } else if (argsStr.size() >= 1) {
                std::string value = argsStr[0];

                return "Serial.write(" + value + ");";
            }
        }

        if (funcName == "read") {
            if (argsStr.size() >= 1) {
                std::string pin = argsStr[0];

                if (pin.length() >= 2 && pin[0] == 'A' && isdigit(pin[1])) {
                    return "analogRead(" + pin + ")";
                } else {
                    return "digitalRead(" + pin + ")";
                }
            } else {
                return "Serial.read();";
            }
        }

        if (funcName == "wait" && argsStr.size() >= 1) {
            std::string fullArg = "";

            for (const auto& arg : argsStr) {
                fullArg += arg;
            }

            return "delay(" + parseTime(fullArg) + ");";
        }

        if (funcName == "start" && argsStr.size() >= 1) {
            return "Serial.begin(" + argsStr[0] + ");";
        }

        if (funcName == "print" && argsStr.size() >= 1) {
            auto* literal = dynamic_cast<LiteralNode*>(arguments[0].get());
            bool isStringLiteral = literal && literal->token.type == TokenType::STRING;

            if (isStringLiteral) {
                return "Serial.print(F(" + argsStr[0] + "));";
            }

            return "Serial.print(" + argsStr[0] + ");";
        }

        if (funcName == "println" && argsStr.size() >= 1) {
            auto* literal = dynamic_cast<LiteralNode*>(arguments[0].get());
            bool isStringLiteral = literal && literal->token.type == TokenType::STRING;

            if (isStringLiteral) {
                return "Serial.println(F(" + argsStr[0] + "));";
            }

            return "Serial.println(" + argsStr[0] + ");";
        }

        if (funcName == "available") {
            return "Serial.available()";
        }

        if (funcName == "availableForWrite") {
            return "Serial.availableForWrite()";
        }

        if (funcName == "end") {
            return "Serial.end()";
        }

        if (funcName == "find" && argsStr.size() >= 2) {
            std::string target = argsStr[0];
            std::string length = argsStr[1];

            return "Serial.find(" + target + ", " + length + ")";
        }

        if (funcName == "findUntil" && argsStr.size() >= 2) {
            std::string target = argsStr[0];
            std::string terminal = argsStr[1];

            return "Serial.find(" + target + ", " + terminal + ")";
        }

        if (funcName == "waitUntilSend") {
            return "Serial.flush();";
        }

        if (funcName == "parseFloat") {
            if (argsStr.size() >= 2) {
                return "Serial.parseFloat(" + argsStr[0] + ", " + argsStr[1] + ");";
            } else if (argsStr.size() == 1) {
                return "Serial.parseFloat(" + argsStr[0] + ");";
            } else {
                return "Serial.parseFloat();";
            }
        }

        if (funcName == "parseInt") {
            if (argsStr.size() >= 2) {
                return "Serial.parseInt(" + argsStr[0] + ", " + argsStr[1] + ");";
            } else if (argsStr.size() == 1) {
                return "Serial.parseInt(" + argsStr[0] + ");";
            } else {
                return "Serial.parseInt();";
            }
        }

        if (funcName == "peek") {
            return "Serial.peek();";
        }

        if (funcName == "scale" && argsStr.size() >= 5) {
            return "map(" + argsStr[0] + ", " + argsStr[1] + ", " + argsStr[2] + ", " + argsStr[3] + ", " + argsStr[4] + ")";
        }

        std::string fallbackArgs = "";
        for (size_t i = 0; i < argsStr.size(); ++i) {
            fallbackArgs += argsStr[i] + (i < argsStr.size() - 1 ? ", " : "");
        }
        
        return funcName + "(" + fallbackArgs + ");";
    }
};

class UnaryOpNode : public ExpressionNode {
private:
    Token op;
    std::unique_ptr<ExpressionNode> right;

public:
    UnaryOpNode(Token op, std::unique_ptr<ExpressionNode> right)
        : op(op), right(std::move(right)) {}

    std::string toCpp() override {
        std::string opStr = op.value;

        if (opStr == "not") {
            opStr = "!";
        }

        std::string rightStr = right->toCpp();

        return opStr + rightStr;
    }
};

class BinaryOpNode : public ExpressionNode {
private:
    Token op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;

public:
    BinaryOpNode(std::unique_ptr<ExpressionNode> left, Token op, std::unique_ptr<ExpressionNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

public:
    std::string getVariableName() override {
        if (auto var = dynamic_cast<LiteralNode*>(left.get())) {
            if (var->token.type == TokenType::SYMBOL) {
                return var->token.value;
            }
        }
        
        if (auto var = dynamic_cast<LiteralNode*>(right.get())) {
            if (var->token.type == TokenType::SYMBOL) {
                return var->token.value;
            }
        }

        return "";
    }

public:
    std::string toCpp() override {
        std::string opStr = op.value;

        if (opStr == "and") {
            opStr = "&&";
        } else if (opStr == "or") {
            opStr = "||";
        }

        std::string leftStr = left->toCpp();
        std::string rightStr = right->toCpp();

        if ((opStr == "==" || opStr == "!=")) {
            if (leftStr == "Serial.read()" || leftStr.find("command") != std::string::npos) {
                if (rightStr.size() == 3 && rightStr.front() == '"' && rightStr.back() == '"') {
                    rightStr = "'" + rightStr.substr(1, 1) + "'";
                }
            }
        }

        return leftStr + " " + opStr + " " + rightStr;
    }
};

class CompoundAssignNode : public ASTNode {
private:
    std::string op;
    std::string name;
    std::string value;

public:
    CompoundAssignNode(std::string name, std::string op, std::string value)
        : name(std::move(name)), op(std::move(op)), value(std::move(value)) {}

public:
    std::string toCpp() override {
        if (op == "++") {
            return name + "++;";
        }

        if (op == "--") {
            return name + "--;";
        }

        return name + " " + op + " " + value + ";";
    }
};

class IfNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> thenBody;
    std::vector<std::unique_ptr<ASTNode>> elseBody;

public:
    IfNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> body)
        : condition(std::move(cond)), thenBody(std::move(body)) {}

public:
    void setElseBody(std::vector<std::unique_ptr<ASTNode>> body) {
        elseBody = std::move(body);
    }

public:
    std::string toCpp() override {
        std::string code = "if (" + condition->toCpp() + ") {\n";
        
        for (const auto& node : thenBody) {
            code += node->toCpp() + "\n";
        }

        code += "}";

        if (!elseBody.empty()) {
            if (elseBody.size() == 1 && dynamic_cast<IfNode*>(elseBody[0].get())) {
                std::string inner = elseBody[0]->toCpp();
                code += "else " + inner;
            } else {
                code += "else {\n";
                
                for (const auto& node : elseBody) {
                    code += node->toCpp() + "\n";
                }
                
                code += "}";
            }
        }

        return code;
    }
};

class UserFuncNode : public ASTNode {
private:
    std::string funcName;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    UserFuncNode(const std::string& name, std::vector<std::string> params, std::vector<std::unique_ptr<ASTNode>> body)
        : funcName(name), params(std::move(params)), body(std::move(body)) {}

public:
    std::string toCpp() override {
        std::string result = "void " + funcName + "(";

        for (size_t i = 0; i < params.size(); i++) {
            result += "int " + params[i] + (i < params.size() - 1 ? ", " : "");
        }

        result += ") {\n";

        for (const auto& node : body) {
            result += node->toCpp() + "\n";
        }

        result += "}\n";

        return result;
    }
};

class ReturnNode : public ASTNode {
private:
    std::string value;

public:
    ReturnNode(const std::string& val)
        : value(val) {}

public:
    std::string toCpp() override {
        return "return " + value + ";";
    }
};

class EveryNode : public ASTNode {
private:
    int id;
    std::string interval;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    EveryNode(std::string interval, std::vector<std::unique_ptr<ASTNode>> body)
        : interval(interval), body(std::move(body)) {

        static int counter = 0;
        id = counter++;
    }

public:
    std::string toCpp() override {
        interval = parseTime(interval);

        std::string timerName = "_every_timer_" + std::to_string(id);
        std::string code = "static unsigned long " + timerName + " = 0;\n";

        code += "if (millis() - " + timerName + " >= " + interval + ") {\n";
        code += timerName + " = millis();\n";

        for (const auto& node : body) {
            code += node->toCpp() + "\n";
        }

        code += "}";

        return code;
    }
};

class WhileNode : public ASTNode {
private:
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    WhileNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<ASTNode>> b)
        : condition(std::move(cond)), body(std::move(b)) {}

public:
    std::string toCpp() override {
        std::string result = "while (" + condition->toCpp() + ") {\n";

        for (const auto& node : body) {
            result += node->toCpp() + "\n";
        }

        result += "}\n";

        return result;
    }
};

class ForNode : public ASTNode {
private:
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    ForNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<ASTNode>> b)
        : condition(std::move(cond)), body(std::move(b)) {}

public:
    std::string toCpp() override {
        std::string condStr = condition->toCpp();
        std::string varName = condition->getVariableName();

        std::string op = "++"; 
        std::string startValue = "0";

        if (condStr.find(">") != std::string::npos) {
            op = "--";
        }

        std::string result = "for (uint32_t " + varName + " = " + startValue + "; " + condStr + "; " + varName + op + ") {\n";

        for (const auto& node : body) {
            result += node->toCpp() + "\n";
        }

        result += "}\n";

        return result;
    }
};

class ForRangeNode : public ASTNode {
private:
    std::string varName;
    std::unique_ptr<ExpressionNode> start;
    std::unique_ptr<ExpressionNode> stop;
    std::unique_ptr<ExpressionNode> step;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    ForRangeNode(std::string varName, std::unique_ptr<ExpressionNode> start, std::unique_ptr<ExpressionNode> stop, std::unique_ptr<ExpressionNode> step, std::vector<std::unique_ptr<ASTNode>> body)
        : varName(std::move(varName)), start(std::move(start)), stop(std::move(stop)), step(std::move(step)), body(std::move(body)) {}

public:
    std::string toCpp() override {
        std::string result = "for (uint32_t " + varName + " = " + start->toCpp() + "; " + varName + " < " + stop->toCpp() + "; " + varName + " += " + step->toCpp() + ") {";

        for (const auto& node : body) {
            result += node->toCpp() + "\n";
        }

        result += "}\n";

        return result;
    }
};

class RepeatNode : public ASTNode {
private:
    int id;
    std::string count;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    RepeatNode(std::string count, std::vector<std::unique_ptr<ASTNode>> body)
        : count(count), body(std::move(body)) {

        static int counter = 0;
        id = counter++;
    }

public:
    std::string toCpp() override {
        std::string iterVar = "_loop_i_" + std::to_string(id);
        std::string code = "for (int " + iterVar + " = 0; " + iterVar + " < " + count + "; " + iterVar + "++) {\n";
        
        for (const auto& node : body) {
            code += node->toCpp() + "\n";
        }

        code += "}";

        return code;
    }
};

class GroupNode : public ExpressionNode {
public:
    std::unique_ptr<ASTNode> expr;

public:
    GroupNode(std::unique_ptr<ASTNode> e)
        : expr(std::move(e)) {}

public:
    std::string toCpp() override {
        return "(" + expr->toCpp() + ")";
    }
};

class OnPressNode : public ASTNode {
private:
    int id;
    std::string pin;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    OnPressNode(std::string pin, std::vector<std::unique_ptr<ASTNode>> body)
        : pin(pin), body(std::move(body)) {

        static int counter = 0;
        id = counter++;
        inputPins.insert(pin);
    }

public:
    std::string toCpp() override {
        std::string timerVar = "_btn_timer_" + std::to_string(id);
        std::string lastVar = "_btn_last_" + std::to_string(id);
        std::string currVar = "_btn_curr_" + std::to_string(id);
        std::string code = "static unsigned long " + timerVar + " = 0;\n";

        code += "static bool " + lastVar + " = HIGH;\n";
        code += "bool " + currVar + " = digitalRead(" + pin + ");\n";
        code += "if (" + currVar + " == LOW && " + lastVar + " == HIGH && (millis() - " + timerVar + " > 200)) {\n";
        code += timerVar + " = millis();\n";

        for (const auto& node : body) {
            code += node->toCpp() + "\n";
        }

        code += "}\n";
        code += lastVar + " = " + currVar + ";";

        return code;
    }
};

class MethodCallNode : public ExpressionNode {
private:
    std::string objectName;
    std::unique_ptr<ASTNode> methodCall;

public:
    MethodCallNode(std::string obj, std::unique_ptr<ASTNode> method)
        : objectName(obj), methodCall(std::move(method)) {}

public:
    std::string toCpp() override {
        std::string methodStr = methodCall->toCpp();

        if (methodStr.find("Serial.") == 0) {
            methodStr = methodStr.substr(7);
        }

        std::string finalObjectName = objectName;

        if (objectName == "serial") {
            finalObjectName = "Serial";
        }

        return finalObjectName + "." + methodStr;
    }
};

class ControlTransferStatementsNode : public ASTNode {
private:
    std::string statement;

public:
    ControlTransferStatementsNode(const std::string& statement)
        : statement(std::move(statement)) {}

public:
    std::string toCpp() override {
        if (statement == "break") {
            return "break;";
        } else if (statement == "continue") {
            return "continue;";
        }

        return "";
    }
};