@echo off
REM ====================================================================
REM  Batch B Execution Script: Muhammad Rashad Wahedna's Physical Phone
REM ====================================================================

set PATH=%~dp0..;%PATH%
cd /d "%~dp0.."

echo ====================================================================
echo  Executing Batch B on Muhammad Rashad Wahedna's Physical Android Phone
echo  Matrix: 4 Configurations x 5 Exit Modes x 5 Repeats = 100 Trials
echo ====================================================================

if not exist experiment_runner.exe (
    call scripts\build.bat
)

if not exist output mkdir output

echo [DEPLOY] Verifying target_app on connected phone...
adb push target_app_arm64 /data/local/tmp/target_app >nul 2>&1
adb shell chmod 755 /data/local/tmp/target_app >nul 2>&1

echo [RUNNER] Executing Rashad's trial batch over ADB...
experiment_runner.exe --repeats 5 --adb --operator RASHAD --device-tag RASHAD_ANDROID --output output\experiment_runs_rashad.csv

echo.
echo ====================================================================
echo [DONE] Rashad's Batch B complete!
echo Output saved to: output\experiment_runs_rashad.csv
echo ====================================================================
pause
