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

#include "types.hpp"

#include <map>

class SymbolTable {
private:
    std::map<std::string, FunctionSymbol> functions;
    std::vector<std::map<std::string, VariableSymbol>> variableScopes;

public:
    SymbolTable();

public:
    void enterScope();
    void exitScope();

public:
    bool declareVariable(const VariableSymbol& symbol);
    bool declareFunction(const FunctionSymbol& function);

public:
    const VariableSymbol* lookupVariable(const std::string& name) const;
    const FunctionSymbol* lookupFunction(const std::string& name) const;
};