# Persistence of Sensitive Data on Android After App Closure

**Course:** Applied Operating Systems, Cyber Defense and AI Agents  
**Institution:** Habib University  
**Instructors:** Dr. Bilal Wajid, Z. Iqbal, S. Tahir, A. S. Shaikh  
**Team Members:**
- **Muhammad Ayub Ansari** (Target & Instrumentation Lead)
- **Muhammad Rashad Wahedna** (Cross-Validation & Statistical Lead)
- **Mahad Khan Khoso** (Storage Forensics & Data Compilation Lead)

---

## What Is This Project About?

When you close a mobile app on your phone, you probably assume that any sensitive data—like an authentication token, a session key, or temporary cache files—gets cleaned up or wiped. 

In this project, we wanted to test whether that assumption is actually true in the real world. 

Instead of using virtual emulators (following our faculty's directive: *"Emulation won't work"*), we built a native low-level test system in pure C and tested it directly on **four physical Android smartphones** across **400 empirical trials**.

We wrote unique cryptographic tokens (called "canaries") across five different Android storage locations:
1. Internal App Files (`/data/data/.../files`)
2. SQLite Database (`.db` B-Tree tables)
3. SharedPreferences (`.xml` configuration files)
4. Temporary App Cache (`/cache`)
5. Android System Logs (`logcat` ring buffer)

Then, we triggered five different ways an Android app can close or be terminated by the operating system:
* **Graceful Exits:** Pressing the **Home** button or the **Back** button.
* **Abrupt Exits:** Using **Force Stop** in Settings, letting the Android **Low Memory Killer (LMKD)** kill the app under memory pressure, or triggering an unhandled **Crash (SIGSEGV)**.

---

## Key Findings (The Short Version)

1. **The Kernel Preemption Trap (Why Mitigations Fail):**
   * Under normal exits (Home or Back), user-space cleanup code runs properly, and sensitive data can be wiped.
   * But whenever the app is closed abruptly (**Force Stop**, **Low Memory Killer**, or a **Crash**), **cleanup code NEVER runs** ($T_{cleanup} = 0.000\text{ ms}$ across 100% of our runs).
   * **Why?** Because the Linux kernel kills the app instantly using `SIGKILL`. Android's standard cleanup methods (like `onDestroy()`) are completely bypassed. As a result, **100% of unencrypted data remains sitting on physical flash storage indefinitely**.
2. **The "Plain Cleanup" Illusion in SQLite:**
   * Many developers think running `DELETE FROM secrets;` erases their data. It does not!
   * SQLite simply marks the database page as "unallocated" without overwriting the bytes. We built a custom binary carver tool in pure C (`forensic_carver.exe`) that recovered the plaintext canary tokens directly from the raw database file, even after SQL reported the table was completely empty.
3. **Hardware Storage Controller Differences:**
   * We found that physical flash storage hardware makes a big difference. Phones with **UFS flash storage** (like the Samsung Galaxy A34 and A15) executed secure overwrites **~38% to 42% faster** than phones with budget **eMMC flash storage** (like the Redmi 13).
4. **Android 17 Validation:**
   * Testing on Rashad's Samsung Galaxy A56 running **Android 17 (API level 37)** showed that this kernel preemption vulnerability remains identical even on the latest generation of Android.

---

## Team Roles & Individual Contributions

To cover the full scope of operating system internals and forensics, our work was split into three distinct technical leads:

* **Muhammad Ayub Ansari (Target & Instrumentation Lead):**
  * Wrote the low-level test application (`src/target_app.c`) and exit mode controller (`src/exit_controller.c`) in pure C.
  * Statically compiled the ARM64 Linux ELF binary (`target_app_arm64`) for direct execution on Android.
  * Executed 300 physical device trials across three phones (Redmi 13, Samsung Galaxy A34, and Samsung Galaxy A15).
  * Analyzed Linux Page Cache delayed writebacks and Flash Translation Layer (FTL) wear-leveling mechanics.
* **Mahad Khan Khoso (Storage Forensics & Data Compilation Lead):**
  * Built the raw binary SQLite free-page forensic carver (`src/forensic_carver.c`) to extract lingering tokens from unallocated B-Tree database pages.
  * Created the multi-device data aggregation pipeline in pure C (`src/data_compiler.c`) to merge and validate all 400 trials.
  * Authored the storage forensics and database internals sections of the research report.
* **Muhammad Rashad Wahedna (Cross-Validation & Statistical Lead):**
  * Executed the 100-trial cross-validation batch on his physical Samsung Galaxy A56 5G running Android 17 (API 37).
  * Built the automated statistical and Shannon Information Entropy plotting tool (`scripts/generate_report_charts.py`).
  * Generated publication-quality vector SVG charts and authored the cross-device comparative analysis.

---

## Directory & File Structure

Here is a simple map of what every folder and file in this repository does:

```text
├── docs/                                 # Complete documentation & research guides
│   ├── EMPIRICAL_DATA_ANALYSIS_AND_FINDINGS.md  # Master research paper & data analysis
│   ├── AYUB_WORK_SPEC.md                 # Ayub's instrumentation & kernel documentation
│   ├── MAHAD_HANDOFF_GUIDE.md            # Mahad's forensics & data compilation guide
│   ├── RASHAD_RUN_GUIDE.md               # Rashad's Android 17 & plotting guide
│   └── EMULATOR_VS_PHONE.md              # Technical comparison of real silicon vs. QEMU
│
├── src/                                  # Pure C source code
│   ├── common.h                          # Shared structs, constants, and timing definitions
│   ├── target_app.c                      # Core C test harness writing to 5 storage layers
│   ├── exit_controller.c                 # C tool triggering the 5 OS termination modes
│   ├── experiment_runner.c               # Master C orchestrator running the trial matrix
│   ├── data_compiler.c                   # Multi-device CSV reconciliation pipeline in C
│   ├── forensic_carver.c                 # Low-level SQLite B-Tree free-page carver in C
│   ├── sqlite3.c                         # Official SQLite single-file C amalgamation
│   └── sqlite3.h                         # Official SQLite header
│
├── output/                               # Empirical datasets & generated figures
│   ├── master_compiled_experiment_data.csv # Master dataset (400 physical trials, 4 devices)
│   ├── cross_device_statistical_summary.txt# Formatted statistical summary of all runs
│   ├── experiment_runs_ayub.csv          # 100 runs on Xiaomi Redmi 13 (HyperOS / eMMC)
│   ├── experiment_runs_samsung1.csv      # 100 runs on Samsung Galaxy A34 (OneUI 6 / UFS)
│   ├── experiment_runs_samsung2.csv      # 100 runs on Samsung Galaxy A15 (OneUI 6 / UFS)
│   ├── experiment_runs_rashad.csv        # 100 runs on Samsung Galaxy A56 (Android 17)
│   └── figures/                          # Generated publication-quality vector charts
│       ├── chart_setup_latency_comparison.svg
│       ├── chart_secure_wipe_latency.svg
│       └── chart_shannon_entropy_comparison.svg
│
├── scripts/                              # Automated batch scripts & utilities
│   ├── generate_report_charts.py         # Python script generating SVG charts & entropy
│   ├── compile_data_mahad.bat            # 1-click script to run the C data compiler
│   ├── run_batch_samsung1.bat            # Automated batch runner for Samsung Phone 1
│   ├── run_batch_samsung2.bat            # Automated batch runner for Samsung Phone 2
│   ├── run_batch_ayub.bat                # Automated batch runner for Redmi 13
│   ├── run_batch_rashad.bat              # Automated batch runner for Galaxy A56
│   ├── build.bat                         # Compiles all C tools for Windows using GCC
│   └── build_android_arm64.bat           # Cross-compiles target app for Android ARM64
│
├── adb.exe, AdbWinApi.dll, AdbWinUsbApi.dll # Portable Google Platform-Tools for ADB
├── target_app_arm64                      # Pre-compiled static ARM64 ELF binary for Android
├── target_app.exe                        # Pre-compiled Windows harness
├── exit_controller.exe                   # Pre-compiled Windows exit controller
├── experiment_runner.exe                 # Pre-compiled Windows experiment orchestrator
├── data_compiler.exe                     # Pre-compiled Windows multi-device compiler
├── forensic_carver.exe                   # Pre-compiled Windows SQLite forensic carver
├── README.md                             # This self-explanatory guide
└── .gitignore                            # Clean Git repository filter
```

---

## How to Run the Project (Step-by-Step)

You do **not** need to install complex dependencies or set up an entire Android development environment to inspect or verify our results. Everything is pre-compiled and ready to run.

### Option 1: Quick Offline Demo (Takes 1 Minute on Any Laptop)

You can test our tools directly on your PC without needing a physical phone connected:

#### 1. Test the SQLite Forensic Carver:
See for yourself how our C carver recovers data from unallocated SQLite pages after a deletion:
```cmd
forensic_carver.exe test_database.db CANARY_
```
The tool will inspect the raw bytes of the database file and print a hex dump showing the exact byte offset and page number where lingering tokens survived.

#### 2. Re-compile the Multi-Device Master Dataset:
To run Mahad's C compiler that aggregates all 400 trials across all four devices:
```cmd
data_compiler.exe
```
This automatically updates `output/master_compiled_experiment_data.csv` and produces `output/cross_device_statistical_summary.txt`.

#### 3. Generate the Charts & Shannon Entropy Analysis:
To run Rashad's Python plotting script that reads the master dataset and generates vector charts:
```cmd
python scripts\generate_report_charts.py
```
This will generate clean SVG charts in `output/figures/` which you can open in any web browser or Word document.

---

### Option 2: Running on a Physical Android Phone over ADB

If you have an Android phone and want to run live tests on physical silicon:

1. **Enable Developer Options on the Phone:**
   * Go to **Settings** > **About Phone**.
   * Tap **Build Number** 7 times until you see *"You are now a developer!"*.
2. **Enable USB Debugging:**
   * Go to **Settings** > **Developer Options** > Turn on **USB Debugging**.
3. **Connect the Phone via USB:**
   * Plug the phone into your computer.
   * In the USB popup on your phone, choose **"Transferring files / Android Auto"**.
   * If an *"Allow USB debugging?"* prompt appears, check **"Always allow from this computer"** and tap **Allow**.
4. **Deploy and Run:**
   * Double-click `scripts\run_batch_samsung1.bat` (or whichever script corresponds to your device).
   * The script will push `target_app_arm64` to `/data/local/tmp/target_app` on the phone, execute the automated test matrix, measure latencies, and save results to a CSV file in `output/`.

---

### Option 3: Compiling from Source Code

If you want to re-compile the C source code yourself:

* **To compile all Windows C tools (GCC / MinGW):**
  ```cmd
  scripts\build.bat
  ```
* **To cross-compile the Android ARM64 binary (`target_app_arm64`):**
  ```cmd
  scripts\build_android_arm64.bat
  ```
  *(Requires WSL or an `aarch64-linux-gnu-gcc` cross-compiler).*

---

## Physical Hardware Testbed Details

| Property | Device 1 | Device 2 | Device 3 | Device 4 (Rashad) |
| :--- | :---: | :---: | :---: | :---: |
| **Model** | Xiaomi Redmi 13 | Samsung Galaxy A34 5G | Samsung Galaxy A15 | Samsung Galaxy A56 5G |
| **Model Number**| `2404ARN45A` | `SM-A346E` | `SM-A155F` | `SM-A566B` |
| **OS Version** | Android 14 (HyperOS)| Android 14 (OneUI 6.0)| Android 14 (OneUI 6.0)| **Android 17 (API 37)** |
| **Processor** | Helio G91-Ultra | Dimensity 1080 | Helio G99 (`mt6789`) | Exynos 1580 Octa |
| **Storage** | **eMMC 5.1** Flash | **UFS 2.2** Flash | **UFS 2.2** Flash | **UFS 3.1** Flash |
| **Trials Run** | **100 runs** | **100 runs** | **100 runs** | **100 runs** |
| **Positive Control**| **100% Passed** | **100% Passed** | **100% Passed** | **100% Passed** |

---

## Academic & Course References

* **Main Research Findings & Data Report:** [docs/EMPIRICAL_DATA_ANALYSIS_AND_FINDINGS.md](docs/EMPIRICAL_DATA_ANALYSIS_AND_FINDINGS.md)
* **Master Compiled CSV Dataset (400 Trials):** [output/master_compiled_experiment_data.csv](output/master_compiled_experiment_data.csv)
* **Statistical Digest:** [output/cross_device_statistical_summary.txt](output/cross_device_statistical_summary.txt)
* **Forensic SQLite Carver Source:** [src/forensic_carver.c](src/forensic_carver.c)
* **Course Policy:** Conducted in accordance with Habib University Applied OS guidelines.
