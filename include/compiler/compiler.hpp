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

std::string getComputerCoreNumber();
std::filesystem::path getTempSketchDir();

bool installLibraries();
bool compileCode();
bool uploadCode();
bool runMelloCompiler(int argc, char* argv[]);