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
#include <memory>

class ASTNode;

class ModuleLoader {
private:
    std::string sourceDir;
    std::string moduleName;

public:
    ModuleLoader(const std::string& moduleName, const std::string& sourceDir)
        : moduleName(std::move(moduleName)), sourceDir(std::move(sourceDir)) {}

public:
    std::vector<std::unique_ptr<ASTNode>> load();
};