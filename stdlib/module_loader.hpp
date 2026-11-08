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

#include "compiler.hpp"

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

class ASTNode;

class ModuleLoader {
private:
    std::string sourceDir;
    std::string moduleName;

private:
    inline static std::vector<std::string> loadedModules;

public:
    ModuleLoader(std::string moduleName, std::string sourceDir)
        : moduleName(std::move(moduleName)), sourceDir(std::move(sourceDir)) {}

private:
    std::filesystem::path searchForFile(const std::filesystem::path& directory, const std::string& fileName);

public:
    std::vector<std::unique_ptr<ASTNode>> load();
};