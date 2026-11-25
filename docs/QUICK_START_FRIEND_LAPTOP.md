# Quick Start Guide: Running Experiments on Friend's Laptop

**For:** Muhammad Rashad Wahedna  
**Device:** Physical Android Phone B  
**Goal:** Run Batch B (100 trials) and produce `output\experiment_runs_rashad.csv`  

---

## 1. Resolved Pre-Flight Checks
- **ARM64 Native Binary (`target_app_arm64`):** Now **pre-compiled and included directly in the folder**. You do **NOT** need Android NDK, GCC, or compilation tools on your laptop. It is statically linked with SQLite and ready to execute on Android hardware.
- **Process Verification & Process Names:** The runner now inspects the Android Linux kernel directly via `/proc/<PID>` and `pidof target_app`, correctly tracking the native process state and clearing `/data/local/tmp/test_app_data/*` between runs.

---

## 2. Prerequisites (Only Tool Needed)
You only need **ADB** (Android Debug Bridge):
- If you have Android Studio or platform-tools installed, ADB is already present.
- If not, download [Google Platform-Tools for Windows](https://dl.google.com/android/repository/platform-tools-latest-windows.zip), unzip it, and add it to your PATH (or drop `adb.exe`, `AdbWinApi.dll`, and `AdbWinUsbApi.dll` into this project folder).

---

## 3. Phone Setup (1 Minute)
1. Plug your Android phone into your laptop via USB.
2. Enable **Developer Options**: Go to **Settings > About Phone** and tap **Build Number** 7 times.
3. Enable **USB Debugging**: Go to **Settings > Developer Options** and toggle **USB Debugging** ON.
4. When prompted on your phone screen, tap **"Always allow from this computer"**.

---

## 4. How to Run Batch B (2 Clicks)

1. Double-click:
   ```cmd
   scripts\setup_physical_phone.bat
   ```
   *(Wakes the screen, keeps it awake, and disables animation jitter)*

2. Double-click:
   ```cmd
   scripts\run_batch_rashad.bat
   ```
   *(Automatically pushes `target_app_arm64` to your phone and runs all 100 trials over ADB)*

---

## 5. Send Results Back
When finished, send:
`output\experiment_runs_rashad.csv`  
back to Ayub and Mahad. Mahad will run `compile_data_mahad.bat` to combine your data with Ayub's data for the final report!
