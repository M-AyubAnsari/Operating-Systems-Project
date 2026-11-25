@echo off
REM ====================================================================
REM  Execution Wrapper for 100-Trial Experiment Matrix
REM  Author: Muhammad Ayub Ansari
REM ====================================================================

echo ====================================================================
echo  Launching Full 100-Trial Matrix (4 Configs x 5 Exits x 5 Repeats)
echo ====================================================================

if not exist target_app.exe (
    echo [INFO] target_app.exe not found. Building binaries first...
    call scripts\build.bat
)

if not exist output mkdir output

experiment_runner.exe --repeats 5 --output output\experiment_runs.csv
echo.
echo [DONE] Experiments finished. Results stored in output\experiment_runs.csv
pause
