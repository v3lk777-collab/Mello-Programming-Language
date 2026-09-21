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

DataType SemanticAnalyzer::mapStringToDataType(const std::string& typeStr) {
    if (typeStr == "int" || typeStr == "uint8_t" || typeStr == "int16_t" || typeStr == "uint16_t" || typeStr == "int32_t" || typeStr == "uint32_t") {
        return DataType::INTEGER;
    }

    if (typeStr == "float") {
        return DataType::FLOAT;
    }

    if (typeStr == "String" || typeStr == "const char*") {
        return DataType::STRING;
    }

    if (typeStr == "bool") {
        return DataType::BOOLEAN;
    }

    return DataType::UNKNOWN;
}