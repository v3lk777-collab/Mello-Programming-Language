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

#include "utils.hpp"
#include "lexer.hpp"
#include "visitor.hpp"
#include "module_loader.hpp"

#include <array>
#include <sstream>
#include <string_view>

class ASTNode {
public:
    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return {};
    }

public:
    virtual ~ASTNode() = default;

public:
    virtual void accept(ASTVisitor& visitor) = 0;
};

class ExpressionNode : public ASTNode {
public:
    virtual std::string getVariableName() {
        return "";
    };

public:
    virtual ~ExpressionNode() = default;
};

class UseNode : public ASTNode {
private:
    std::string libraryName;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    UseNode(std::string libraryName, int currentLine, int currentColumn, std::string source)
        : libraryName(std::move(libraryName)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    std::string getLibraryName() const noexcept {
        return libraryName;
    }

    std::string getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }
public:
    void accept(ASTVisitor& visitor) override {
        return;
    }
};

class ArrayNode : public ASTNode {
private:
    std::string arrayName;
    std::vector<std::unique_ptr<ExpressionNode>> arrayMembers;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    ArrayNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> arrayMembers, int currentLine, int currentColumn, std::string source)
        : arrayName(std::move(name)), arrayMembers(std::move(arrayMembers)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getArrayName() const noexcept {
        return arrayName;
    }

    const std::vector<std::unique_ptr<ExpressionNode>>& getArrayMembers() const noexcept {
        return arrayMembers;
    }

    const std::string& getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
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
    const std::string& getArrayName() const noexcept {
        return arrayName;
    }

    const std::unique_ptr<ExpressionNode>& getIndexExpression() const noexcept {
        return indexExpression;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ArrayAssignNode : public ExpressionNode {
private:
    std::string arrayName;
    std::unique_ptr<ExpressionNode> indexExpression;
    std::unique_ptr<ExpressionNode> valueExpression;

public:
    ArrayAssignNode(std::string arrayName, std::unique_ptr<ExpressionNode> indexExpression, std::unique_ptr<ExpressionNode> valueExpression)
        : arrayName(std::move(arrayName)), indexExpression(std::move(indexExpression)), valueExpression(std::move(valueExpression)) {}

public:
    const std::string& getArrayName() const noexcept {
        return arrayName;
    }

    const std::unique_ptr<ExpressionNode>& getIndexExpression() const noexcept {
        return indexExpression;
    }

    const std::unique_ptr<ExpressionNode>& getValueExpression() const noexcept {
        return valueExpression;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ArrayBuiltInFunctionsNode : public ExpressionNode {
private:
    std::string arrayName;
    std::string functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

public:
    ArrayBuiltInFunctionsNode(std::string arrayName, std::string functionName, std::vector<std::unique_ptr<ExpressionNode>> arguments)
        : arrayName(std::move(arrayName)), functionName(std::move(functionName)), arguments(std::move(arguments)) {}

public:
    const std::string& getArrayName() const noexcept {
        return arrayName;
    }

    const std::string& getFunctionName() const noexcept {
        return functionName;
    }

    const std::vector<std::unique_ptr<ExpressionNode>>& getArguments() const noexcept {
        return arguments;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class FunctionNode : public ASTNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> body;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    FunctionNode(std::string name, std::vector<std::unique_ptr<ASTNode>> funcBody, int currentLine, int currentColumn, std::string source)
        : funcName(std::move(name)), body(std::move(funcBody)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getFunctionName() const noexcept {
        return funcName;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    const std::string& getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class LiteralNode : public ExpressionNode {
private:
    Token token;

public:
    LiteralNode(Token token)
        : token(token) {}

public:
    const Token& getToken() const noexcept {
        return token;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class FunctionCallNode : public ExpressionNode {
private:
    bool isStatement;
    std::string funcName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> arguments, bool isStatement, int currentLine, int currentColumn, std::string source)
        : funcName(std::move(name)), arguments(std::move(arguments)), isStatement(isStatement), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getFunctionName() const noexcept {
        return funcName;
    }

    const std::vector<std::unique_ptr<ExpressionNode>>& getArguments() const noexcept { 
        return arguments; 
    }

    bool getIsStatement() const noexcept {
        return isStatement;
    }

    const int getFuncArgumentsSize() const noexcept {
        return arguments.size();;
    }

    std::string getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class BuiltInFunctionCallNode : public ExpressionNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    BuiltInFunctionCallNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> args, int currentLine, int currentColumn, std::string source)
        : funcName(std::move(name)), arguments(std::move(args)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getFunctionName() const noexcept {
        return funcName;
    }

    const std::vector<std::unique_ptr<ExpressionNode>>& getArguments() const noexcept {
        return arguments;
    }

    const std::string& getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class SerialFunctionsCallNode : public ExpressionNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    SerialFunctionsCallNode(std::string funcName, std::vector<std::unique_ptr<ExpressionNode>> arguments, int currentLine, int currentColumn, std::string source)
        : funcName(std::move(funcName)), arguments(std::move(arguments)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getFunctionName() const noexcept {
        return funcName;
    }

    const std::vector<std::unique_ptr<ExpressionNode>>& getArguments() const noexcept {
        return arguments;
    }

    const std::string& getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class MethodCallNode : public ExpressionNode {
private:
    std::string objectName;
    std::unique_ptr<ASTNode> methodCall;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    MethodCallNode(std::string object, std::unique_ptr<ASTNode> method, int currentLine, int currentColumn, std::string source)
        : objectName(std::move(object)), methodCall(std::move(method)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getObjectName() const noexcept {
        return objectName;
    }

    ASTNode* getMethodCall() {
        return methodCall.get();
    }

    std::string getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class VarAssignNode : public ASTNode {
public:
    std::string name;
    std::string type;
    bool isConstantVar;
    TokenType val_type;
    std::string raw_value;
    std::unique_ptr<ASTNode> value;
    bool declaration = true;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    VarAssignNode(std::string name, std::unique_ptr<ASTNode> value, std::string raw, TokenType val_type, bool isConstantVar, int currentLine, int currentColumn, std::string source) 
        : name(std::move(name)), value(std::move(value)), raw_value(std::move(raw)), val_type(val_type), isConstantVar(isConstantVar), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getName() const noexcept {
        return name;
    }

    ASTNode* getValue() const noexcept {
        return value.get();
    }

    TokenType getValueType() const noexcept {
        return val_type;
    }

    const std::string& getRawValue() const noexcept {
        return raw_value;
    }

    bool getIsConstantVar() const noexcept {
        return isConstantVar;
    }

    bool isDeclaration() const noexcept {
        return declaration;
    }

    void setIsDeclaration(bool value) noexcept {
        declaration = value;
    }

    const std::string& getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class UnaryOpNode : public ExpressionNode {
private:
    Token op;
    std::unique_ptr<ExpressionNode> right;

public:
    UnaryOpNode(Token op, std::unique_ptr<ExpressionNode> right)
        : op(op), right(std::move(right)) {}

public:
    const Token& getOp() const noexcept {
        return op;
    }

    ExpressionNode* getRight() const noexcept { 
        return right.get(); 
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
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
    Token getOp() const noexcept { 
        return op; 
    }

    ExpressionNode* getLeft() const noexcept { 
        return left.get();
    }

    ExpressionNode* getRight() const noexcept { 
        return right.get(); 
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class BitwiseNode : public ExpressionNode {
private:
    std::string op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;

public:
    BitwiseNode(std::unique_ptr<ExpressionNode> left, std::string op, std::unique_ptr<ExpressionNode> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

public:
    const std::string& getOp() const noexcept { 
        return op; 
    }

    ExpressionNode* getLeft() const noexcept { 
        return left.get();
    }

    ExpressionNode* getRight() const noexcept { 
        return right.get(); 
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class CompoundAssignNode : public ASTNode {
private:
    std::string op;
    std::string name;
    std::unique_ptr<ExpressionNode> value;

public:
    CompoundAssignNode(std::string name, std::string op, std::unique_ptr<ExpressionNode> value = nullptr)
        : name(std::move(name)), op(std::move(op)), value(std::move(value)) {}

public:
    const std::string& getOp() const noexcept {
        return op;
    }

    const std::string& getName() const noexcept {
        return name;
    }

    ExpressionNode* getValue() const noexcept {
        return value.get();
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
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
    const std::unique_ptr<ASTNode>& getCondition() const noexcept {
        return condition;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getThenBody() const noexcept {
        return thenBody;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getElseBody() const noexcept {
        return elseBody;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &thenBody, &elseBody };
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ReturnNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> value;

public:
    ReturnNode(std::unique_ptr<ASTNode> value)
        : value(std::move(value)) {}

public:
    ASTNode* getValue() const noexcept {
        return value.get();
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class UserFuncNode : public ASTNode {
private:
    std::string funcName;
    std::string userReturnType;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<ASTNode>> body;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    UserFuncNode(const std::string& name, std::vector<std::string> params, const std::string& userReturnType, std::vector<std::unique_ptr<ASTNode>> body, int currentLine, int currentColumn, std::string source)
        : funcName(std::move(name)), params(std::move(params)), userReturnType(std::move(userReturnType)), body(std::move(body)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getFunctionName() const noexcept {
        return funcName;
    }

    const std::vector<std::string>& getFuncParams() const noexcept {
        return params;
    }

    const std::vector<std::unique_ptr<ASTNode>> &getBody() const noexcept {
        return body;
    }

    const std::string& getUserReturnType() const noexcept {
        return userReturnType;
    }

    std::string getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class EveryNode : public ASTNode {
private:
    int id;
    std::string interval;
    std::vector<std::unique_ptr<ASTNode>> body;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    EveryNode(std::string interval, std::vector<std::unique_ptr<ASTNode>> body, int currentLine, int currentColumn, std::string source)
        : interval(std::move(interval)), body(std::move(body)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {

        static int containser = 0;
        id = containser++;
    }

public:
    int getID() const noexcept {
        return id;
    }

    const std::string& getInterval() {
        return interval;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &body };
    }

    std::string getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
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
    const std::unique_ptr<ExpressionNode>& getCondition() const noexcept {
        return condition;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &body };
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ForNode : public ASTNode {
private:
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    ForNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<ASTNode>> body)
        : condition(std::move(cond)), body(std::move(body)) {}

public:
    const std::unique_ptr<ExpressionNode> &getCondition() const noexcept {
        return condition;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &body };
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ForRangeNode : public ASTNode {
private:
    std::string varName;
    std::unique_ptr<ExpressionNode> start;
    std::unique_ptr<ExpressionNode> stop;
    std::unique_ptr<ExpressionNode> step;
    std::vector<std::unique_ptr<ASTNode>> body;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    ForRangeNode(std::string varName, std::unique_ptr<ExpressionNode> start, std::unique_ptr<ExpressionNode> stop, std::unique_ptr<ExpressionNode> step, std::vector<std::unique_ptr<ASTNode>> body, int currentLine, int currentColumn, std::string source)
        : varName(std::move(varName)), start(std::move(start)), stop(std::move(stop)), step(std::move(step)), body(std::move(body)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getVarName() const noexcept {
        return varName;
    }

    const std::unique_ptr<ExpressionNode>& getStart() const noexcept {
        return start;
    }

    const std::unique_ptr<ExpressionNode>& getStop() const noexcept {
        return stop;
    }

    const std::unique_ptr<ExpressionNode>& getStep() const noexcept {
        return step;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &body };
    }

    const std::string& getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class RepeatNode : public ASTNode {
private:
    int id;
    std::string count;
    std::vector<std::unique_ptr<ASTNode>> body;

public:
    RepeatNode(std::string count, std::vector<std::unique_ptr<ASTNode>> body)
        : count(std::move(count)), body(std::move(body)) {

        static int counter = 0;
        id = counter++;
    }

public:
    int getID() const noexcept {
        return id;
    }

    const std::string& getCount() const noexcept {
        return count;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &body };
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class GroupNode : public ExpressionNode {
public:
    std::unique_ptr<ASTNode> expression;

public:
    GroupNode(std::unique_ptr<ASTNode> expression)
        : expression(std::move(expression)) {}


public:
    ExpressionNode* getExpression() const noexcept {
        return dynamic_cast<ExpressionNode*>(expression.get());
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
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

        static int containser = 0;
        id = containser++;

        inputPullUpPins.insert(pin);
    }

public:
    int getID() const noexcept {
        return id;
    }

    const std::string& getPin() const noexcept {
        return pin;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

    virtual std::vector<const std::vector<std::unique_ptr<ASTNode>>*> getChildBodies() const noexcept {
        return { &body };
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ControlTransferStatementsNode : public ASTNode {
private:
    std::string statement;

public:
    ControlTransferStatementsNode(const std::string& statement)
        : statement(std::move(statement)) {}

public:
    const std::string& getStatement() const noexcept {
        return statement;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class TypeConversionCallNode : public ExpressionNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    TypeConversionCallNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> args, int currentLine, int currentColumn, std::string source)
        : funcName(std::move(name)), arguments(std::move(args)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getFunctionName() const noexcept {
        return funcName;
    }

    const std::vector<std::unique_ptr<ExpressionNode>>& getArguments() const noexcept {
        return arguments;
    }

    std::string getSource() const noexcept {
        return source;
    }

    int getCurrentDeclaredLine() const noexcept {
        return currentLine;
    }

    int getCurrentDeclaredColumn() const noexcept {
        return currentColumn;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

class ModuleNode : public ASTNode {
private:
    std::string moduleName;
    std::vector<std::unique_ptr<ASTNode>> body;

private:
    int currentLine;
    int currentColumn;
    std::string source;

public:
    ModuleNode(std::string moduleName, std::vector<std::unique_ptr<ASTNode>> body, int currentLine, int currentColumn, std::string source)
        : moduleName(std::move(moduleName)), body(std::move(body)), currentLine(currentLine), currentColumn(currentColumn), source(std::move(source)) {}

public:
    const std::string& getModuleName() const noexcept {
        return moduleName;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getBody() const noexcept {
        return body;
    }

public:
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};