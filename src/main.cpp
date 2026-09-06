/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
*/

#include "compiler.hpp"

int main(int argc, char* argv[]) {
    Compiler compiler;

    if (!compiler.run(argc, argv)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}