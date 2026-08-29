// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

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