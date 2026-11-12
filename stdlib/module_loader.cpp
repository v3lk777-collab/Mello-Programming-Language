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

#include "module_loader.hpp"

#include "ast.hpp"
#include "utils.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "parser.hpp"
#include "error_handler.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::filesystem::path ModuleLoader::searchForFile(const std::filesystem::path& directory, const std::string& fileName) {
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().filename() == fileName) {
                return entry.path();
            }
        }

        ErrorHandler::report("Can't find the file:", fileName, 0, 0, "");
    } catch (const std::filesystem::filesystem_error& error) {
        ErrorHandler::report("Error while searching for the file:", error.what(), 0, 0, "", true);
    }

    ErrorHandler::report("Can't find the file:", fileName, 0, 0, "", true);

    return {};
}

std::vector<std::unique_ptr<ASTNode>> ModuleLoader::load() {
    if (std::find(loadedModules.begin(), loadedModules.end(), moduleName) != loadedModules.end()) {
        return {};
    }

    std::filesystem::path path;

    if (stdLibs.contains(moduleName)) {
        if (stdProtocol.contains(moduleName)) {
            path = "../stdlib/protocols/" + moduleName + "/" + moduleName + ".mello";
        } else {
            path = "../stdlib/" + moduleName + "/" + moduleName + ".mello";
        }
    } else {
        path = searchForFile(sourceDir, moduleName + ".mello");
    }

    std::ifstream moduleFile(path);

    if (!moduleFile.is_open()) {
        ErrorHandler::report("Can't open '" + moduleName + "' standerd library file", "", 0, 0, "");
    }

    loadedModules.push_back(moduleName);

    std::stringstream buffer;

    buffer << moduleFile.rdbuf();

    std::string sourceCode = buffer.str();

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    if (ErrorHandler::hasErrors()) {
        ErrorHandler::showErrors();

        return {};
    }
    
    std::vector<std::unique_ptr<ASTNode>> program;

    try {
        Parser parser(tokens, sourceCode);

        program = parser.parse();

        if (ErrorHandler::hasErrors()) {
            ErrorHandler::showErrors();

            return {};
        }
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << "\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
    }

    std::set<std::string> dependencies;

    for (const auto& node : program) {
        auto* useNode = dynamic_cast<UseNode*>(node.get());

        if (!useNode) {
            continue;
        }

        const std::string dependencyName = useNode->getLibraryName();

        if (dependencyName == moduleName || arduinoLibraries.contains(dependencyName)) {
            continue;
        }

        dependencies.insert(dependencyName);
    }

    std::vector<std::unique_ptr<ASTNode>> result;

    for (const auto& dependency : dependencies) {
        ModuleLoader dependencyLoader(dependency, sourceDir);

        auto dependencyNodes = dependencyLoader.load();

        for (auto& node : dependencyNodes) {
            result.push_back(std::move(node));
        }
    }

    userDefinedModulesNames.insert(moduleName);

    if (stdProtocol.contains(moduleName)) {

        for (auto& node : program) {
            result.push_back(std::move(node));
        }

        return result;
    }

    result.push_back(std::make_unique<ModuleNode>(moduleName, std::move(program), 0, 0, ""));

    return result;
}