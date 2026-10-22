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
    static int errorCount;

public:
    static void report(std::string message, std::string tokenValue, int lineNumber, int columnNumber, std::string source);
    static bool hasErrors();
    static void printErrors();
};

inline int ErrorHandler::errorCount = 0;
inline static std::vector<Error> errors;