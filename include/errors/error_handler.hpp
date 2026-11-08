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

#include "error.hpp"

#include <vector>

class ErrorHandler {
private:
    inline static std::vector<Error> errors;

public:
    static void report(std::string message, std::string tokenValue, int lineNumber, int columnNumber, std::string source, bool shouldExitTheProgram = false);

private:
    static void printError(const std::string& message, const std::string& tokenValue, int lineNumber, int columnNumber, const std::string& source);

public:
    static bool hasErrors();
    static void showErrors();
};