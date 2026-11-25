@echo off
REM ====================================================================
REM  Deploy C Binaries to Physical Android Phone & Run Dry Run
REM  Author: Muhammad Ayub Ansari
REM ====================================================================

set PATH=%~dp0..;%PATH%
cd /d "%~dp0.."

echo ====================================================================
echo  Deploying C Binaries to Physical Android Phone (/data/local/tmp/)
echo ====================================================================

if not exist target_app_arm64 (
    echo [INFO] target_app_arm64 not found. Attempting build...
    call scripts\build_android_arm64.bat
)

if not exist target_app_arm64 (
    echo [ERROR] target_app_arm64 binary does not exist. Please compile first.
    pause
    exit /b 1
)

echo [1/3] Pushing target_app_arm64 to /data/local/tmp/target_app...
adb push target_app_arm64 /data/local/tmp/target_app
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] adb push failed. Make sure phone is connected and authorized.
    pause
    exit /b 1
)

echo [2/3] Setting executable permissions (chmod 755)...
adb shell chmod 755 /data/local/tmp/target_app

echo [3/3] Running dry-run trial on physical device...
adb shell "/data/local/tmp/target_app --config 0 --exit-mode 2 --canary 'CANARY_DEVICE_DRYRUN_PASS' --base-dir '/data/local/tmp/test_app_data' --verify-active"

echo.
echo ====================================================================
echo  Deployment complete! Physical device is verified and ready.
echo ====================================================================
pause
