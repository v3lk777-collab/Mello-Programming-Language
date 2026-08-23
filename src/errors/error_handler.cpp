// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive
// academic or commercial purpose is strictly prohibited without
// explicit written permission from the author.

#include "error_handler.hpp"

#include <vector>
#include <sstream>
#include <cstdlib>
#include <iostream>

void ErrorHandler::report(const std::string &message, const std::string &tokenValue, int lineNumber, int columnNumber, const std::string &source) {
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

        std::string prefix = "  " + std::to_string(lineNumber) + " | ";

        std::cerr << prefix << lineContent << "\n";
        std::cerr << std::string(prefix.length(), ' ');

        for (int i = 1; i < columnNumber; i++) {
            std::cerr << ' ';
        }

        size_t caretLength = displayToken.empty() ? 1 : displayToken.length();
        std::cerr << std::string(caretLength, '^');

        std::cerr << '\n';
    }

    exit(EXIT_FAILURE);
}

bool ErrorHandler::hasErrors() {
    return errorCount > 0;
}