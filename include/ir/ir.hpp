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

#include <deque>
#include <string>
#include <vector>
#include <variant>

enum class IRType {
    VOID,
    INTEGER,
    FLOAT,
    STRING,
    CHARACTER,
    BOOLEAN
};

enum class IROpcode {
    CONSTANT,
    ADD,
    SUB,
    MUL,
    DIV,
    EQUAL,
    EQUALITY,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    NOT_EQUAL,
    LOAD,
    CALL,
    STORE,
    RETURN,
    FUNCTION,
    BRANCH,
    CONDITIONAL_BRANCH
};

struct IRValue {
    int id;
    IRType type;
    bool isConstant;
    std::variant<int, float, std::string, bool, char> value;
};

struct IRInstruction {
    IRValue result;
    IROpcode opcode;
    std::string name;
    std::string target;
    std::string falseTarget;
    std::vector<IRValue> operands;
};

struct IRBlock {
    std::string name;
    std::vector<IRInstruction> instructions;
};

struct IRFunction {
    std::string name;
    IRType returnType;
    std::deque<IRBlock> blocks;
    std::vector<std::string> parameters;
};