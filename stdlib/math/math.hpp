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

#include "ast.hpp"

class Math : public ASTNode {
private:
    std::string function;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

public:
    Math(std::string function, std::vector<std::unique_ptr<ExpressionNode>> arguments)
        : function(std::move(function)), arguments(std::move(arguments)) {}

public:
    std::string toCpp() override {
        std::string cppFunction;

        if (function == "sqrt" && arguments.size() == 1) {
            cppFunction = "std::sqrt";
        }
        else if (function == "sin" && arguments.size() == 1) {
            cppFunction = "std::sin";
        }
        else if (function == "cos" && arguments.size() == 1) {
            cppFunction = "std::cos";
        }
        else if (function == "tan" && arguments.size() == 1) {
            cppFunction = "std::tan";
        }
        else if (function == "asin" && arguments.size() == 1) {
            cppFunction = "std::asin";
        }
        else if (function == "acos" && arguments.size() == 1) {
            cppFunction = "std::acos";
        }
        else if (function == "atan" && arguments.size() == 1) {
            cppFunction = "std::atan";
        }
        else if (function == "floor" && arguments.size() == 1) {
            cppFunction = "std::floor";
        }
        else if (function == "ceil" && arguments.size() == 1) {
            cppFunction = "std::ceil";
        }
        else if (function == "round" && arguments.size() == 1) {
            cppFunction = "std::round";
        }
        else if (function == "abs" && arguments.size() == 1) {
            cppFunction = "std::abs";
        }
        else if (function == "exp" && arguments.size() == 1) {
            cppFunction = "std::exp";
        }
        else if (function == "log" && arguments.size() == 1) {
            cppFunction = "std::log";
        }
        else if (function == "log10" && arguments.size() == 1) {
            cppFunction = "std::log10";
        }
        else {
            return function;
        }

        std::string result = cppFunction + "(";

        for (size_t i = 0; i < arguments.size(); ++i) {
            result += arguments[i]->toCpp();

            if (i + 1 < arguments.size()) {
                result += ", ";
            }
        }

        result += ")";

        return result;
    }
};