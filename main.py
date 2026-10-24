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

import os
import platform
import subprocess

RED = '\033[31m'
GREEN = '\033[32m'
RESET = '\033[0m'

def main() -> None:
    subprocess.run(["cls" if platform.system() == "Windows" else "clear"], shell=True)

    currentOS = platform.system()

    if currentOS == "Windows":
        VCVARS_PATH = r"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat"

        if not os.path.exists(VCVARS_PATH):
            print(f"[{RED}!{RESET}] {RED}Error{RESET}: Visual Studio vcvars64.bat not found at '{VCVARS_PATH}'. Please ensure Visual Studio is installed.")
            return

        result = subprocess.run(f"\"{VCVARS_PATH}\" && cmake --build cmake", shell=True, text=True, capture_output=True)

        if result.returncode != 0:
            print(f"[{RED}!{RESET}] {RED}Error{RESET}: CMake build failed with error code {result.returncode}.")

            if result.stdout:
                print(f"    -----> {result.stdout}")

            if result.stderr:
                print(f"    -----> {result.stderr}")

            return
        else:
            print(f"[{GREEN}!{RESET}] {GREEN}Success{RESET}: CMake build completed successfully.")

        subprocess.run("py test_runner.py", shell=True)
    elif currentOS == "Linux":
        result = subprocess.run(f"cmake --build cmake", shell=True, text=True, capture_output=True)

        if result.returncode != 0:
            print(f"[{RED}!{RESET}] {RED}Error{RESET}: CMake build failed with error code {result.returncode}.")

            if result.stdout:
                print(f"    -----> {result.stdout}")

            if result.stderr:
                print(f"    -----> {result.stderr}")

            return
        else:
            print(f"[{GREEN}!{RESET}] {GREEN}Success{RESET}: CMake build completed successfully.")

        subprocess.run("python3 test_runner.py", shell=True)
    else:
        print(f"[{RED}!{RESET}] {RED}Error{RESET}: Unsupported operating system: {currentOS}")

if __name__ == "__main__":
    main()