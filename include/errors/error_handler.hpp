/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
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