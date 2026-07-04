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
#include <iostream>

class ErrorHandler {
private:
    static int errorCount;

public:
    static void report(const std::string& message, const std::string& tokenValue, int lineNumber, const std::string& source) {

        std::vector<std::string> lines;
        std::stringstream ss(source);
        std::string line;

        while (std::getline(ss, line, '\n')) {
            lines.push_back(line);
        }

        std::cerr << "\033[1;31m[Mello Error]\033[0m Line " << line << ": " << message << "\n";
        
        if (lineNumber > 0 && lineNumber <= (int)lines.size()) {
            std::cerr << "  " << lineNumber << " | " << lines[lineNumber - 1] << "\n";
            std::cerr << "    " << std::string(std::to_string(lineNumber).size() + 3, ' ') << "^" << "\n";
        }

        exit(EXIT_FAILURE);
    }

    static bool hasErrors() {
        return errorCount > 0;
    }
};

int ErrorHandler::errorCount = 0;