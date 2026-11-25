@echo off
REM ====================================================================
REM  Batch Execution Script: Samsung Galaxy Phone 1
REM ====================================================================

set PATH=%~dp0..;%PATH%
cd /d "%~dp0.."

set ANDROID_SERIAL=
adb disconnect >nul 2>&1

echo ====================================================================
echo  Executing 100-Trial Matrix on Samsung Galaxy Phone 1
echo  Operator: Muhammad Ayub Ansari
echo ====================================================================

if not exist output mkdir output

echo [DEPLOY] Verifying target_app on connected Samsung phone...
adb push target_app_arm64 /data/local/tmp/target_app >nul 2>&1
adb shell chmod 755 /data/local/tmp/target_app >nul 2>&1

echo [RUNNER] Executing trial batch over ADB...
experiment_runner.exe --repeats 5 --adb --operator AYUB --device-tag SAMSUNG_GALAXY_1 --output output\experiment_runs_samsung1.csv

echo.
echo ====================================================================
echo [DONE] Samsung Phone 1 batch complete!
echo Output saved to: output\experiment_runs_samsung1.csv
echo ====================================================================
pause
