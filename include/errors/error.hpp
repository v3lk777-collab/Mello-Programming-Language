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
#include <utility>

struct Error {
    std::string message;
    std::string tokenValue;

    int lineNumber;
    int columnNumber;
    std::string source;

    Error(std::string message, std::string tokenValue, int lineNumber, int columnNumber, std::string source)
        : message(std::move(message)), tokenValue(std::move(tokenValue)), lineNumber(lineNumber), columnNumber(columnNumber), source(std::move(source)) {}
};