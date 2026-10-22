"""
 * Mello Programming Language
 *
 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
 *
 * This source code is proprietary and confidential. Unauthorized copying, 
 * modification, distribution, or use of this file for any academic, 
 * commercial, or competitive purpose, via any medium, is strictly 
 * prohibited without the express written permission of the author.
"""

import subprocess
from pathlib import Path

RED = '\033[31m'
GREEN = '\033[32m'
RESET = '\033[0m'

COMPILER = Path(__file__).parent / "cmake/mello.exe"
FILE_EXTENSION = ".mello"
TESTS_DIR = Path(__file__).parent / "tests"

def runFile(file_path: Path):
    if not file_path.exists():
        print(f"Error: File '{file_path}' does not exist.")
        return

    if file_path.suffix != FILE_EXTENSION:
        print(f"Error: File '{file_path}' is not a valid Mello source file.")
        return

    try:
        result = subprocess.run(f"{str(COMPILER)} {str(file_path)} --no-compile", cwd=COMPILER.parent, capture_output=True, text=True, shell=True)

        if result.stdout:
            print(result.stdout)

        if result.stderr:
            print(f"{RED}Errors:{RESET}", result.stderr)

    except Exception as error:
        print(f"{RED}An error occurred while running the compiler: {error}{RESET}")

def main():
    if not TESTS_DIR.exists():
        print(f"{RED}Error: Tests directory '{TESTS_DIR}' does not exist.{RESET}")
        return

    for file_path in TESTS_DIR.glob(f"*{FILE_EXTENSION}"):
        print(f"{GREEN}Running test: {file_path.name}{RESET}")
        runFile(file_path)

if __name__ == "__main__":
    main()