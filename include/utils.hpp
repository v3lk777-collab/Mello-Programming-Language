// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>

inline auto parseTime = [] (std::string timeVal) -> std::string {
    timeVal.erase(std::remove(timeVal.begin(), timeVal.end(), ' '), timeVal.end());
    timeVal.erase(std::remove(timeVal.begin(), timeVal.end(), '\"'), timeVal.end());

    if (timeVal.size() > 1) {
        std::string numPart = timeVal.substr(0, timeVal.size() - 1);
        
        try {
            long long num = std::stoll(numPart);

            switch(timeVal.back()) {
                case 's':
                    return std::to_string(num * 1000);
                
                case 'm':
                    return std::to_string(num * 60000);
                
                case 'h':
                    return std::to_string(num * 3600000);
            }
        } catch (const std::invalid_argument& error) {
            std::cerr << "\n[Error] Mello Compiler: Expected a valid number in wait(), but got: '" << numPart << "'\n";
            exit(1);
        } catch (const std::out_of_range& error) {
            std::cerr << "\n[Error] Mello Compiler: The time value '" << numPart << "' is too large!\n";
            exit(1); 
        } catch (...) {
            std::cerr << "\n[Error] Mello Compiler: Unknown error while parsing time.\n";
            return timeVal;
        }
    }

    return timeVal;
};
