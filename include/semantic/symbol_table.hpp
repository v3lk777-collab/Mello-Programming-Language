/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
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