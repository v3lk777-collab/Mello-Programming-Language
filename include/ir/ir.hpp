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

#include <memory>
#include <string>
#include <vector>

enum class IRLiteralType {
    INTEGER,
    FLOAT,
    STRING,
    CHARACTER,
    BOOLEAN
};

enum class IROp {
    MINUS,
    PLUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    EQUALITY,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    LOGICAL_NOT,
    INCREMENT,
    DECREMENT
};

class IRNode {
public:
    virtual ~IRNode() = default;
};

class IRUseNode : public IRNode {
private:
    std::string libraryName;

public:
    const std::string& getLibraryName() const {
        return libraryName;
    }

public:
    IRUseNode(std::string libraryName)
        : libraryName(std::move(libraryName)) {}
};

class IRArrayNode : public IRNode {
private:
    std::string arrayName;
    std::vector<std::string> arrayMembers;

public:
    const std::string& getArrayName() const {
        return arrayName;
    }

    const std::vector<std::string>& getArrayMembers() const {
        return arrayMembers;
    }

public:
    IRArrayNode(std::string arrayName, std::vector<std::string> arrayMembers)
        : arrayName(std::move(arrayName)), arrayMembers(std::move(arrayMembers)) {}
};

class IRArrayIndexNode : public IRNode {
private:
    std::string arrayName;
    std::unique_ptr<IRNode> indexExpression;

public:
    const std::string& getArrayName() const {
        return arrayName;
    }

    const std::unique_ptr<IRNode>& getIndexExpression() const {
        return indexExpression;
    }

public:
    IRArrayIndexNode(std::string arrayName, std::unique_ptr<IRNode> indexExpression)
        : arrayName(std::move(arrayName)), indexExpression(std::move(indexExpression)) {}
};

class IRArrayAssignNode : public IRNode {
private:
    std::string arrayName;
    std::unique_ptr<IRNode> indexExpression;
    std::unique_ptr<IRNode> valueExpression;

public:
    const std::string& getArrayName() const {
        return arrayName;
    }

    const std::unique_ptr<IRNode>& getIndexExpression() const {
        return indexExpression;
    }

    const std::unique_ptr<IRNode>& getValueExpression() const {
        return valueExpression;
    }

public:
    IRArrayAssignNode(std::string arrayName, std::unique_ptr<IRNode> indexExpression, std::unique_ptr<IRNode> valueExpression)
        : arrayName(std::move(arrayName)), indexExpression(std::move(indexExpression)), valueExpression(std::move(valueExpression)) {}
};

class IRFunctionNode : public IRNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    const std::string& getFunctionName() const {
        return funcName;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IRFunctionNode(std::string funcName, std::vector<std::unique_ptr<IRNode>> body)
        : funcName(std::move(funcName)), body(std::move(body)) {}
};

class IRLiteralNode : public IRNode {
private:
    std::string value;
    IRLiteralType type;

public:
    const std::string& getValue() const {
        return value;
    }

    IRLiteralType getType() const {
        return type;
    }

public:
    IRLiteralNode(std::string value, IRLiteralType type)
        : value(std::move(value)), type(type) {}
};

class IRFunctionCallNode : public IRNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<IRNode>> arguments;

public:
    const std::string& getFunctionName() const {
        return funcName;
    }

    const std::vector<std::unique_ptr<IRNode>>& getArguments() const {
        return arguments;
    }

public:
    IRFunctionCallNode(std::string funcName, std::vector<std::unique_ptr<IRNode>> arguments)
        : funcName(std::move(funcName)), arguments(std::move(arguments)) {}
};

class IRBuiltInFunctionCallNode : public IRNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<IRNode>> arguments;

public:
    const std::string& getFunctionName() const {
        return funcName;
    }

    const std::vector<std::unique_ptr<IRNode>>& getArguments() const {
        return arguments;
    }

public:
    IRBuiltInFunctionCallNode(std::string funcName, std::vector<std::unique_ptr<IRNode>> arguments)
        : funcName(std::move(funcName)), arguments(std::move(arguments)) {}
};

class IRSerialFunctionsCallNode : public IRNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<IRNode>> arguments;

public:
    const std::string& getFunctionName() const {
        return funcName;
    }

    const std::vector<std::unique_ptr<IRNode>>& getArguments() const {
        return arguments;
    }

public:
    IRSerialFunctionsCallNode(std::string funcName, std::vector<std::unique_ptr<IRNode>> arguments)
        : funcName(std::move(funcName)), arguments(std::move(arguments)) {}
};

class IRMethodCallNode : public IRNode {
private:
    std::string objectName;
    std::unique_ptr<IRNode> methodCall;

public:
    const std::string& getObjectName() const {
        return objectName;
    }

    const std::unique_ptr<IRNode>& getMethodCall() const {
        return methodCall;
    }

public:
    IRMethodCallNode(std::string objectName, std::unique_ptr<IRNode> methodCall)
        : objectName(std::move(objectName)), methodCall(std::move(methodCall)) {}
};

class IRVarAssignNode : public IRNode {
private:
    std::string name;
    bool isConstantVar;
    std::string rawValue;
    IRLiteralType valueType;
    std::unique_ptr<IRNode> value;

public:
    const std::string& getName() const {
        return name;
    }

    const std::unique_ptr<IRNode>& getValue() const {
        return value;
    }

    IRLiteralType getValueType() const {
        return valueType;
    }

    const std::string& getRawValue() const {
        return rawValue;
    }

    bool getIsConstantVar() const {
        return isConstantVar;
    }

public:
    IRVarAssignNode(std::string name, std::unique_ptr<IRNode> value, std::string rawValue, IRLiteralType valueType, bool isConstantVar)
        : name(std::move(name)), value(std::move(value)), rawValue(std::move(rawValue)), valueType(valueType), isConstantVar(isConstantVar) {}
};

class IRUnaryOpNode : public IRNode {
private:
    IROp op;
    std::unique_ptr<IRNode> right;

public:
    IROp getOp() const {
        return op;
    }

    const std::unique_ptr<IRNode>& getRight() const {
        return right;
    }

public:
    IRUnaryOpNode(IROp op, std::unique_ptr<IRNode> right)
        : op(op), right(std::move(right)) {}
};

class IRBinaryOpNode : public IRNode {
private:
    IROp op;
    std::unique_ptr<IRNode> left;
    std::unique_ptr<IRNode> right;

public:
    IROp getOp() const {
        return op;
    }

    const std::unique_ptr<IRNode>& getLeft() const {
        return left;
    }

    const std::unique_ptr<IRNode>& getRight() const {
        return right;
    }

public:
    IRBinaryOpNode(std::unique_ptr<IRNode> left, IROp op, std::unique_ptr<IRNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

class IRCompoundAssignNode : public IRNode {
private:
    IROp op;
    std::string name;
    std::string value;

public:
    IROp getOp() const {
        return op;
    }

    const std::string& getName() const {
        return name;
    }

    const std::string& getValue() const {
        return value;
    }

public:
    IRCompoundAssignNode(std::string name, IROp op, std::string value)
        : name(std::move(name)), op(op), value(std::move(value)) {}
};

class IRIfNode : public IRNode {
private:
    std::unique_ptr<IRNode> condition;
    std::vector<std::unique_ptr<IRNode>> thenBody;
    std::vector<std::unique_ptr<IRNode>> elseBody;

public:
    const std::unique_ptr<IRNode>& getCondition() const {
        return condition;
    }

    const std::vector<std::unique_ptr<IRNode>>& getThenBody() const {
        return thenBody;
    }

    const std::vector<std::unique_ptr<IRNode>>& getElseBody() const {
        return elseBody;
    }

public:
    IRIfNode(std::unique_ptr<IRNode> cond, std::vector<std::unique_ptr<IRNode>> thenBody, std::vector<std::unique_ptr<IRNode>> elseBody)
        : condition(std::move(cond)), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {}
};

class IRReturnNode : public IRNode {
private:
    std::unique_ptr<IRNode> value;

public:
    const std::unique_ptr<IRNode>& getValue() const {
        return value;
    }

public:
    IRReturnNode(std::unique_ptr<IRNode> val)
        : value(std::move(val)) {}
};

class IRUserFuncNode : public IRNode {
private:
    std::string funcName;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    const std::string& getFunctionName() const {
        return funcName;
    }

    const std::vector<std::string>& getParams() const {
        return params;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IRUserFuncNode(std::string funcName, std::vector<std::string> params, std::vector<std::unique_ptr<IRNode>> body)
        : funcName(std::move(funcName)), params(std::move(params)), body(std::move(body)) {}
};

class IREveryNode : public IRNode {
private:
    int id;
    std::string interval;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    int getId() const {
        return id;
    }

    const std::string& getInterval() const {
        return interval;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IREveryNode(int id, std::string interval, std::vector<std::unique_ptr<IRNode>> body)
        : id(id), interval(std::move(interval)), body(std::move(body)) {}
};

class IRWhileNode : public IRNode {
private:
    std::unique_ptr<IRNode> condition;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    const std::unique_ptr<IRNode>& getCondition() const {
        return condition;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IRWhileNode(std::unique_ptr<IRNode> condition, std::vector<std::unique_ptr<IRNode>> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};

class IRForNode : public IRNode {
private:
    std::unique_ptr<IRNode> condition;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    const std::unique_ptr<IRNode>& getCondition() const {
        return condition;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IRForNode(std::unique_ptr<IRNode> condition, std::vector<std::unique_ptr<IRNode>> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};

class IRForRangeNode : public IRNode {
private:
    std::string varName;
    std::unique_ptr<IRNode> start;
    std::unique_ptr<IRNode> stop;
    std::unique_ptr<IRNode> step;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    const std::string& getVarName() const {
        return varName;
    }

    const std::unique_ptr<IRNode>& getStart() const {
        return start;
    }

    const std::unique_ptr<IRNode>& getStop() const {
        return stop;
    }

    const std::unique_ptr<IRNode>& getStep() const {
        return step;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IRForRangeNode(std::string varName, std::unique_ptr<IRNode> start, std::unique_ptr<IRNode> stop, std::unique_ptr<IRNode> step, std::vector<std::unique_ptr<IRNode>> body)
        : varName(std::move(varName)), start(std::move(start)), stop(std::move(stop)), step(std::move(step)), body(std::move(body)) {}
};

class IRRepeatNode : public IRNode {
private:
    int id;
    std::string count;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    int getId() const {
        return id;
    }

    const std::string& getCount() const {
        return count;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IRRepeatNode(int id, std::string count, std::vector<std::unique_ptr<IRNode>> body)
        : id(id), count(std::move(count)), body(std::move(body)) {}
};

class IRGroupNode : public IRNode {
private:
    std::unique_ptr<IRNode> expression;

public:
    const std::unique_ptr<IRNode>& getExpression() const {
        return expression;
    }

public:
    IRGroupNode(std::unique_ptr<IRNode> expression)
        : expression(std::move(expression)) {}
};

class IROnPressNode : public IRNode {
private:
    int id;
    std::string pin;
    std::vector<std::unique_ptr<IRNode>> body;

public:
    int getId() const {
        return id;
    }

    const std::string& getPin() const {
        return pin;
    }

    const std::vector<std::unique_ptr<IRNode>>& getBody() const {
        return body;
    }

public:
    IROnPressNode(int id, std::string pin, std::vector<std::unique_ptr<IRNode>> body)
        : id(id), pin(std::move(pin)), body(std::move(body)) {}
};

class IRControlTransferStatementsNode : public IRNode {
private:
    std::string statement;

public:
    const std::string& getStatement() const {
        return statement;
    }

public:
    IRControlTransferStatementsNode(std::string statement)
        : statement(std::move(statement)) {}
};

class IRTypeConversionCallNode : public IRNode {
private:
    std::string funcName;
    std::vector<std::unique_ptr<IRNode>> arguments;

public:
    const std::string& getFunctionName() const {
        return funcName;
    }

    const std::vector<std::unique_ptr<IRNode>>& getArguments() const {
        return arguments;
    }

public:
    IRTypeConversionCallNode(std::string funcName, std::vector<std::unique_ptr<IRNode>> arguments)
        : funcName(std::move(funcName)), arguments(std::move(arguments)) {}
};