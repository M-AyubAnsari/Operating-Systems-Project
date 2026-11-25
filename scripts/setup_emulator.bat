@echo off
REM ====================================================================
REM  Android Emulator Setup and Configuration Script
REM  Author: Muhammad Ayub Ansari (Target & Instrumentation Lead)
REM ====================================================================

echo ====================================================================
echo  Configuring Android Emulator for OS Persistence Experiments
echo ====================================================================

echo [1/5] Checking ADB connection...
adb devices
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] ADB not detected in PATH. Please ensure Android SDK Platform-Tools is installed.
    pause
    exit /b 1
)

echo [2/5] Requesting root access on emulator (adb root)...
adb root
timeout /t 2 /nobreak >nul

echo [3/5] Disabling window and transition animations for precise timing...
adb shell settings put global window_animation_scale 0.0
adb shell settings put global transition_animation_scale 0.0
adb shell settings put global animator_duration_scale 0.0

echo [4/5] Preparing target test sandbox directories on Android...
adb shell "mkdir -p /data/data/com.osproject.canaryapp/files"
adb shell "mkdir -p /data/data/com.osproject.canaryapp/databases"
adb shell "mkdir -p /data/data/com.osproject.canaryapp/shared_prefs"
adb shell "mkdir -p /data/data/com.osproject.canaryapp/cache"
adb shell "mkdir -p /sdcard/Android/data/com.osproject.canaryapp/files"
adb shell "chmod -R 777 /data/data/com.osproject.canaryapp"

echo [5/5] Emulator successfully configured and primed for experiments!
echo.
pause
