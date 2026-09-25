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

#include "ir_generator.hpp"

int IRGenerator::getID() noexcept {
    return currentID++;
}

void IRGenerator::setCurrentBlock(IRBlock block) {
    currentFunction->blocks.push_back(std::move(block));

    currentBlock = &currentFunction->blocks.back();
}

IRType IRGenerator::mapTokenTypeToIRType(TokenType tokenType) noexcept {
    switch (tokenType) {
    case TokenType::INTEGER:
        return IRType::INTEGER;

    case TokenType::FLOAT:
        return IRType::FLOAT;

    case TokenType::STRING:
        return IRType::STRING;

    case TokenType::CHARACTER:
        return IRType::CHARACTER;

    case TokenType::BOOLEAN:
        return IRType::BOOLEAN;

    default:
        return IRType::VOID;
    }
}

IRType IRGenerator::mapTokenToIRType(Token token) noexcept {
    switch (token.type) {
    case TokenType::INTEGER:
        return IRType::INTEGER;

    case TokenType::FLOAT:
        return IRType::FLOAT;

    case TokenType::STRING:
        return IRType::STRING;

    case TokenType::CHARACTER:
        return IRType::CHARACTER;

    case TokenType::BOOLEAN:
        return IRType::BOOLEAN;

    default:
        return IRType::VOID;
    }
}

IRType IRGenerator::mapDataTypeToIRType(DataType type) noexcept {
    switch (type) {
    case DataType::INTEGER:
        return IRType::INTEGER;

    case DataType::FLOAT:
        return IRType::FLOAT;

    case DataType::STRING:
        return IRType::STRING;

    case DataType::BOOLEAN:
        return IRType::BOOLEAN;

    default:
        return IRType::VOID;
    }
}

IROpcode IRGenerator::mapTokenToIROpcode(Token token) noexcept {
    switch (token.type) {
    case TokenType::PLUS:
        return IROpcode::ADD;

    case TokenType::MINUS:
        return IROpcode::SUB;

    case TokenType::MULTIPLY:
        return IROpcode::MUL;

    case TokenType::DIVIDE:
        return IROpcode::DIV;

    case TokenType::EQUAL:
        return IROpcode::EQUAL;

    case TokenType::EQUALITY:
        return IROpcode::EQUALITY;

    case TokenType::LESS:
        return IROpcode::LESS;

    case TokenType::GREATER:
        return IROpcode::GREATER;

    case TokenType::LESS_EQUAL:
        return IROpcode::LESS_EQUAL;

    case TokenType::GREATER_EQUAL:
        return IROpcode::GREATER_EQUAL;

    case TokenType::NOT_EQUAL:
        return IROpcode::NOT_EQUAL;

    default:
        return IROpcode::CONSTANT;
    }
}