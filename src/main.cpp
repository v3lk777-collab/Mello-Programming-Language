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

#include "compiler.hpp"

int main(int argc, char* argv[]) {
    Compiler compiler;

    if (!compiler.run(argc, argv)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}