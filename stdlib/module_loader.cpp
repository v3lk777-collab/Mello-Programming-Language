// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#include "module_loader.hpp"

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "parser.hpp"
#include "error_handler.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

std::vector<std::unique_ptr<ASTNode>> ModuleLoader::load() {
    std::string path = "../stdlib/"+ moduleName + "/" + moduleName + ".mello";

    std::ifstream moduleFile(path);

    if (!moduleFile.is_open()) {
        ErrorHandler::report("Can't open '" + moduleName + "' standerd library file", "", 0, 0, "");
    }

    std::stringstream buffer;

    buffer << moduleFile.rdbuf();

    std::string sourceCode = buffer.str();

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    
    std::vector<std::unique_ptr<ASTNode>> program;

    try {
        Parser parser(tokens, sourceCode);

        program = parser.parse();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
    }

    return program;
}