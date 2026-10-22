@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > nul

:loop
    echo [*] Run cmake...
    cmake --build cmake
    echo.

    echo [*] Run tests...
    py main.py
    echo.

    echo [*] Done
    echo --------------------------------------------------
    echo [-] Press ENTER to re-run, type 'c' to clear, or close the window to exit

    set "user_input="
    set /p user_input=
    
    if /i "%user_input%"=="c" cls

    goto loop