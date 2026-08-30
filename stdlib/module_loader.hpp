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
#include <memory>

class ASTNode;

class ModuleLoader {
private:
    std::string moduleName;

public:
    ModuleLoader(std::string moduleName)
        : moduleName(std::move(moduleName)) {}

public:
    std::vector<std::unique_ptr<ASTNode>> load();
};