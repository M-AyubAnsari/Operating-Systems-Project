@echo off
REM ====================================================================
REM  Batch A Execution Script: Muhammad Ayub Ansari's Physical Phone
REM ====================================================================

set PATH=%~dp0..;%PATH%
cd /d "%~dp0.."

REM Connect to paired device if using wireless debugging
set ANDROID_SERIAL=192.168.18.3:36925
adb connect 192.168.18.3:36925 >nul 2>&1

echo ====================================================================
echo  Executing Batch A on Muhammad Ayub Ansari's Physical Android Phone
echo  Matrix: 4 Configurations x 5 Exit Modes x 5 Repeats = 100 Trials
echo ====================================================================

if not exist experiment_runner.exe (
    call scripts\build.bat
)

if not exist output mkdir output

echo [DEPLOY] Verifying target_app on connected phone...
adb push target_app_arm64 /data/local/tmp/target_app >nul 2>&1
adb shell chmod 755 /data/local/tmp/target_app >nul 2>&1

echo [RUNNER] Executing Ayub's trial batch over ADB...
experiment_runner.exe --repeats 5 --adb --operator AYUB --device-tag AYUB_ANDROID --output output\experiment_runs_ayub.csv

echo.
echo ====================================================================
echo [DONE] Ayub's Batch A complete!
echo Output saved to: output\experiment_runs_ayub.csv
echo ====================================================================
pause
