@echo off
REM ====================================================================
REM  Data Compilation Script for Mahad Khan Khoso
REM  Compiles Ayub's Phone A Dataset & Rashad's Phone B Dataset into Master
REM ====================================================================

echo ====================================================================
echo  Data Compilation & Cross-Device Aggregation
echo  Lead: Mahad Khan Khoso
echo ====================================================================

if not exist data_compiler.exe (
    call scripts\build.bat
)

if not exist output\experiment_runs_ayub.csv (
    echo [WARNING] output\experiment_runs_ayub.csv not found.
    echo Please make sure Ayub's batch has finished.
)

if not exist output\experiment_runs_rashad.csv (
    echo [WARNING] output\experiment_runs_rashad.csv not found.
    echo Please make sure Rashad's batch has finished.
)

data_compiler.exe --ayub output\experiment_runs_ayub.csv --rashad output\experiment_runs_rashad.csv --out output\master_compiled_experiment_data.csv

echo.
echo ====================================================================
echo [DONE] Compilation complete!
echo Master Dataset: output\master_compiled_experiment_data.csv
echo Statistical Report: output\cross_device_statistical_summary.txt
echo ====================================================================
pause
