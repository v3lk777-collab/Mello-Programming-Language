// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <iostream>

class ErrorHandler {
private:
    static int errorCount;

public:
    static void report(const std::string& message, const std::string& tokenValue, int lineNumber, const std::string& source) {
        errorCount++;

        std::vector<std::string> lines;
        std::stringstream ss(source);
        std::string currentLineContent;

        std::string displayToken = tokenValue;

        if (displayToken == "\n" || displayToken == "\t" || displayToken == "\r") {
            displayToken = "";
        }

        while (std::getline(ss, currentLineContent, '\n')) {
            if (!currentLineContent.empty() && currentLineContent.back() == '\r') {
                currentLineContent.pop_back();
            }

            lines.push_back(currentLineContent);
        }

        std::cerr << "[Error] Line " << lineNumber << ": " << message << (displayToken.empty() ? "" : " '" + displayToken + "'") << "\n";

        if (lineNumber > 0 && lineNumber <= static_cast<int>(lines.size())) {
            std::string lineContent = lines[lineNumber - 1];

            std::cerr << "  " << lineNumber << " | " << lineContent << "\n";

            std::string prefix = std::string(std::to_string(lineNumber).size() + 3, ' ');
            std::cerr << prefix << "^" << "\n";
        }

        exit(EXIT_FAILURE);
    }

    static bool hasErrors() {
        return errorCount > 0;
    }
};

inline int ErrorHandler::errorCount = 0;