// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

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