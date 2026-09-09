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

#include <string>
#include <vector>

enum class DataType {
    INT,
    STRING,
    FLOAT,
    BOOL,
    UNKNOWN,
    ERROR
};

struct VariableSymbol {
    std::string name;
    DataType type;
    bool isConstant;
    int declaredLine;
};

struct FunctionSymbol {
    std::string name;
    std::vector<std::string> paramNames;
    DataType returnType;
};