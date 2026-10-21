@echo off

echo [*] Run cmake...
cmake --build cmake
echo.

echo [*] Run tests...
py main.py
echo.

echo [*] Done

pause