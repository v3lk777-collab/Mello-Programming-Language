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

#include "error_handler.hpp"

#include <set>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

inline std::set<std::string> inputPins;
inline std::set<std::string> outputPins;
inline std::set<std::string> inputPullUpPins;

inline std::set<std::string> floatVariables;
inline std::set<std::string> stringVariables;
inline std::set<std::string> integerVariables;

inline std::set<std::string> nonConstantVariables;

inline std::set<std::string> includedLibraries;
inline std::set<std::string> installedLibraries;

inline std::set<std::string> funcParams;
inline std::set<std::string> parsedVariables;
inline std::set<std::string> declaredVariables;
inline std::set<std::string> reassignedVariables;

inline std::set<std::string> arraysNamesList;

inline std::set<std::string> userDefinedFunctionNames;

inline std::vector<std::string> currentFunctionParams;

inline std::string currentParsingUserFunc = "";
inline std::vector<std::string> currentFuncParamNames;

inline std::map<std::string, std::vector<bool>> userFuncInputParams;
inline std::map<std::string, std::vector<bool>> userFuncOutputParams;

const std::unordered_set<std::string> stdLibs = {
    "math"
};

inline std::set<std::string> includedStdLibs;

inline auto parseTime = [](std::string timeVal, int line, int column, const std::string& source) -> std::string {
    timeVal.erase(std::remove(timeVal.begin(), timeVal.end(), ' '), timeVal.end());
    timeVal.erase(std::remove(timeVal.begin(), timeVal.end(), '\"'), timeVal.end());

    if (timeVal.size() > 1) {
        std::string numPart = timeVal.substr(0, timeVal.size() - 1);

        try {
            long long num = std::stoll(numPart);

            switch(timeVal.back()) {
                case 's':
                    return std::to_string(num * 1000LL);

                case 'm':
                    return std::to_string(num * 60000LL);

                case 'h':
                    return std::to_string(num * 3600000LL);
            }
        } catch (const std::invalid_argument& error) {
            ErrorHandler::report("Expected a valid number in wait(), but got:", numPart, line, column, source);
        } catch (const std::out_of_range& error) {
            ErrorHandler::report("The time value '" + numPart + "' is too large!", "", line, column, source);
        } catch (...) {
            ErrorHandler::report("Unknown error while parsing time:", timeVal, line, column, source);

            return timeVal;
        }
    }

    return timeVal;
};