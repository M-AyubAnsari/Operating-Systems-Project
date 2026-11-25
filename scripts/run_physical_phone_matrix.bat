@echo off
REM ====================================================================
REM  Run Full 100-Trial Matrix on Physical Android Phone via ADB
REM  Author: Muhammad Ayub Ansari
REM ====================================================================

echo ====================================================================
echo  Executing 100-Trial Matrix on Physical Android Device via ADB
echo  Faculty: Dr. Bilal Wajid ("Emulation wont work")
echo ====================================================================

if not exist experiment_runner.exe (
    call scripts\build.bat
)

if not exist output mkdir output

echo [RUNNER] Starting live automated test suite on connected phone...
experiment_runner.exe --repeats 5 --adb --pkg com.osproject.canaryapp --output output\experiment_runs_physical.csv

echo.
echo ====================================================================
echo [DONE] Physical phone experiment finished!
echo Output dataset written to: output\experiment_runs_physical.csv
echo Give this folder and dataset to Mahad for forensic scanning.
echo ====================================================================
pause
