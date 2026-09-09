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
#include <filesystem>

inline std::string melloSourceFileName;

class Compiler {
private:
    std::string boardType = "uno";
    std::filesystem::path sketchDir;

private:
    void printSketchFileCode(std::filesystem::path inoFilePath);

private:
    std::string getComputerCoreNumber();
    std::filesystem::path getTempSketchDir();

private:
    std::string findArduinoBoard();

private:
    bool installLibraries();
    bool compileCode();
    bool uploadCode();
    bool runMelloCompiler(int argc, char* argv[]);

public:
    Compiler() = default;

    bool run(int argc, char* argv[]);
};