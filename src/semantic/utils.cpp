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

#include "semantic_analyzer.hpp"

DataType SemanticAnalyzer::mapTokenTypeToDataType(TokenType type) {
    switch (type) {
        case TokenType::INTEGER:
            return DataType::INTEGER;

        case TokenType::FLOAT:
            return DataType::FLOAT;

        case TokenType::STRING:
            return DataType::STRING;

        case TokenType::CHARACTER:
            return DataType::CHARACTER;

        case TokenType::BOOLEAN:
            return DataType::BOOLEAN;

        default:
            return DataType::ANY;
    }
}

DataType SemanticAnalyzer::mapStringToDataType(const std::string& typeStr) {
    if (typeStr == "int") {
        return DataType::INTEGER;
    } else if (typeStr == "float") {
        return DataType::FLOAT;
    } else if (typeStr == "str") {
        return DataType::STRING;
    } else if (typeStr == "char") {
        return DataType::CHARACTER;
    } else if (typeStr == "bool") {
        return DataType::BOOLEAN;
    } else if (typeStr == "void") {
        return DataType::ANY;
    }

    return DataType::ANY;
}