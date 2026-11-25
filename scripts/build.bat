@echo off
REM ====================================================================
REM  Build Script for Operating Systems Project (Pure C Implementation)
REM  Author: Muhammad Ayub Ansari (Target and Instrumentation Lead)
REM ====================================================================

echo [BUILD] Detecting C Compiler...
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [BUILD] Looking for MSYS2 GCC at C:\msys64\ucrt64\bin\gcc.exe...
    if exist C:\msys64\ucrt64\bin\gcc.exe (
        set CC=C:\msys64\ucrt64\bin\gcc.exe
    ) else (
        echo [ERROR] No C compiler found. Please install GCC or Clang.
        exit /b 1
    )
) else (
    set CC=gcc
)

echo [BUILD] Using Compiler: %CC%
echo [BUILD] Compiling target_app.c...
%CC% -O2 -Wall -static -I../src target_app.c -lsqlite3 -o ..\target_app.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile target_app.c
    exit /b 1
)

echo [BUILD] Compiling exit_controller.c...
%CC% -O2 -Wall -static -I../src exit_controller.c -o ..\exit_controller.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile exit_controller.c
    exit /b 1
)

echo [BUILD] Compiling experiment_runner.c...
%CC% -O2 -Wall -static -I../src experiment_runner.c -o ..\experiment_runner.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile experiment_runner.c
    exit /b 1
)

echo [BUILD] Compiling data_compiler.c (Mahad's Tool)...
%CC% -O2 -Wall -static -I../src data_compiler.c -o ..\data_compiler.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile data_compiler.c
    exit /b 1
)

echo [BUILD] Build successful! Binaries created in root folder:
echo         - target_app.exe
echo         - exit_controller.exe
echo         - experiment_runner.exe
echo         - data_compiler.exe
pause
