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

        while (std::getline(ss, currentLineContent, '\n')) {
            if (!currentLineContent.empty() && currentLineContent.back() == '\r') {
                currentLineContent.pop_back();
            }

            lines.push_back(currentLineContent);
        }

        std::cerr << "\033[1;31m[Error]\033[0m Line " << lineNumber << ": " << message << (tokenValue.empty() ? "" : " '" + tokenValue + "'") << "\n";
        
        if (lineNumber > 0 && lineNumber <= (int)lines.size()) {
            std::string lineContent = lines[lineNumber - 1];
            
            std::cerr << "  " << lineNumber << " | " << lineContent << "\n";
            
            std::string prefix = std::string(std::to_string(lineNumber).size() + 3, ' ');
            std::cerr << "    " << prefix << "^" << "\n";
        }

        exit(EXIT_FAILURE);
    }

    static bool hasErrors() {
        return errorCount > 0;
    }
};

inline int ErrorHandler::errorCount = 0;