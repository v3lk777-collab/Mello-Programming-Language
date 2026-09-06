/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
*/

#include "symbol_table.hpp"

SymbolTable::SymbolTable() {
    variableScopes.push_back({});
}

void SymbolTable::enterScope() {
    variableScopes.push_back({});
}

void SymbolTable::exitScope() {
    variableScopes.pop_back();
}

bool SymbolTable::declareVariable(const VariableSymbol& symbol) {
    auto& currentScope = variableScopes.back();

    if (currentScope.count(symbol.name)) {
        return false;
    }

    currentScope[symbol.name] = symbol;
    return true;
}

bool SymbolTable::declareFunction(const FunctionSymbol& function) {
    if (functions.count(function.name)) {
        return false;
    }

    functions[function.name] = function;
    return true;
}

const VariableSymbol* SymbolTable::lookupVariable(const std::string& name) const {
    for (auto it = variableScopes.rbegin(); it != variableScopes.rend(); ++it) {
        auto found = it->find(name);

        if (found != it->end()) {
            return &found->second;
        }
    }

    return nullptr;
}

const FunctionSymbol* SymbolTable::lookupFunction(const std::string& name) const {
    auto found = functions.find(name);

    if (found == functions.end()) {
        return nullptr;
    }

    return &found->second;
}