@echo off
REM ====================================================================
REM  Cross-Compile Target C App for Physical Android Phone (ARM64)
REM  Author: Muhammad Ayub Ansari
REM ====================================================================

echo ====================================================================
echo  Cross-Compiling C Target App for Physical ARM64 Android Devices
echo ====================================================================

set NDK_FOUND=0

if defined ANDROID_NDK_HOME (
    set NDK_PATH=%ANDROID_NDK_HOME%
    set NDK_FOUND=1
) else if defined NDK (
    set NDK_PATH=%NDK%
    set NDK_FOUND=1
) else (
    REM Check default Android SDK NDK paths
    for /d %%D in ("%LOCALAPPDATA%\Android\Sdk\ndk\*") do (
        set NDK_PATH=%%D
        set NDK_FOUND=1
    )
)

if %NDK_FOUND% EQU 0 (
    echo [WARNING] Android NDK path not automatically detected.
    echo Please set ANDROID_NDK_HOME to your NDK folder, or compile directly using:
    echo aarch64-linux-android34-clang -O2 -Wall -I../src target_app.c -lsqlite3 -llog -o ../target_app_arm64
    pause
    exit /b 1
)

echo [NDK] Found NDK at: %NDK_PATH%
set CLANG_BIN=%NDK_PATH%\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android34-clang.exe

if not exist "%CLANG_BIN%" (
    echo [ERROR] Could not find aarch64 clang at: %CLANG_BIN%
    pause
    exit /b 1
)

echo [BUILD] Compiling target_app.c for aarch64-linux-android...
cd /d "%~dp0..\src"
"%CLANG_BIN%" -O2 -Wall -I. target_app.c -lsqlite3 -llog -o ..\target_app_arm64
if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] ARM64 binary created: target_app_arm64
    echo Ready to deploy to physical device via scripts\deploy_to_phone.bat
) else (
    echo [ERROR] Compilation failed.
)

pause
