// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once

#include "types.hpp"

#include <map>

class SymbolTable {
private:
    std::map<std::string, FunctionSymbol> functions;
    std::vector<std::map<std::string, VariableSymbol>> variableScopes;

public:
    SymbolTable() {
        variableScopes.push_back({});
    }

public:
    void enterScope() {
        variableScopes.push_back({});
    }

    void exitScope() {
        variableScopes.pop_back();
    }

public:
    bool declareVariable(const VariableSymbol& symbol) {
        auto& currentScope = variableScopes.back();

        if (currentScope.count(symbol.name)) {
            return false;
        }

        currentScope[symbol.name] = symbol;
        return true;
    }

    bool declareFunction(const FunctionSymbol& function) {
        if (functions.count(function.name)) {
            return false;
        }

        functions[function.name] = function;
        return true;
    }

public:
    const VariableSymbol* lookupVariable(const std::string& name) const {
        for (auto it = variableScopes.rbegin(); it != variableScopes.rend(); ++it) {
            auto found = it->find(name);
 
            if (found != it->end()) {
                return &found->second;
            }
        }
 
        return nullptr;
    }

    const FunctionSymbol* lookupFunction(const std::string& name) const {
        auto found = functions.find(name);
 
        if (found == functions.end()) {
            return nullptr;
        }
 
        return &found->second;
    }
};