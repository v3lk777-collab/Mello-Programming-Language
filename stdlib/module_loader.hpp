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
    std::string moduleName;

public:
    ModuleLoader(std::string moduleName)
        : moduleName(std::move(moduleName)) {}

public:
    std::vector<std::unique_ptr<ASTNode>> load();
};