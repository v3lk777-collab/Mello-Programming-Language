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

class ErrorHandler {
private:
    static int errorCount;

public:
    static void report(const std::string& message, const std::string& tokenValue, int lineNumber, int columnNumber, const std::string& source);
    static bool hasErrors();
};

inline int ErrorHandler::errorCount = 0;