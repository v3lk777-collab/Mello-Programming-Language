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

#include "error_handler.hpp"

#include "compiler.hpp"

#include <sstream>
#include <cstdlib>
#include <iostream>

void ErrorHandler::report(std::string message, std::string tokenValue, int lineNumber, int columnNumber, std::string source, bool shouldExitTheProgram) {
    if (shouldExitTheProgram) {
        printError(std::move(message), std::move(tokenValue), lineNumber, columnNumber, std::move(source));

        exit(1);
    }

    if (!errors.empty() && (errors.back().lineNumber == lineNumber && errors.back().columnNumber == columnNumber)) {
        return;
    }

    errorCount++;
    errors.emplace_back(std::move(message), std::move(tokenValue), lineNumber, columnNumber, std::move(source));
}

bool ErrorHandler::hasErrors() {
    return !errors.empty();
}

void ErrorHandler::printError(const std::string& message, const std::string& tokenValue, int lineNumber, int columnNumber, const std::string& source) {
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

    std::cerr << "[" << melloSourceFileName << ":" << lineNumber << ":" << columnNumber << "]: " << message << (displayToken.empty() ? "" : " '" + displayToken + "'") << "\n";

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
}

void ErrorHandler::showErrors() {
    for (const auto& error : errors) {
        printError(std::move(error.message), std::move(error.tokenValue), error.lineNumber, error.columnNumber, std::move(error.source));
    }

    std::cerr << "Total errors: " << errorCount << "\n";
}