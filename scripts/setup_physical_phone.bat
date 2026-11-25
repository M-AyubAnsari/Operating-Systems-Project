@echo off
REM ====================================================================
REM  Physical Android Phone Setup Script (ADB Configuration)
REM  Author: Muhammad Ayub Ansari (Target & Instrumentation Lead)
REM ====================================================================

set PATH=%~dp0..;%PATH%
cd /d "%~dp0.."

set ANDROID_SERIAL=

echo ====================================================================
echo  Configuring Physical Android Phone via ADB
echo  Faculty Directive: "You guys are CS major students. Emulation wont work"
echo ====================================================================

echo [1/5] Checking connected physical device...
adb devices
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] ADB is not recognized or device is not detected.
    echo Please make sure:
    echo  1. Phone is connected via USB cable.
    echo  2. 'Developer Options' is enabled on phone (Settings -> About -> tap Build Number 7 times).
    echo  3. 'USB Debugging' is enabled.
    echo  4. You accepted the 'Allow USB debugging' prompt on the phone screen.
    pause
    exit /b 1
)

echo.
echo [2/5] Waking phone screen and keeping display awake while plugged in...
adb shell input keyevent 224
adb shell input keyevent 82
adb shell svc power stayon true

echo [3/5] Disabling animations to avoid UI rendering jitter in timing...
adb shell settings put global window_animation_scale 0.0
adb shell settings put global transition_animation_scale 0.0
adb shell settings put global animator_duration_scale 0.0

echo [4/5] Creating dedicated test sandbox directory in /data/local/tmp/...
adb shell "mkdir -p /data/local/tmp/test_app_data"
adb shell "chmod -R 777 /data/local/tmp/test_app_data"

echo [5/5] Physical device is successfully configured and ready for experiments!
echo.
pause
