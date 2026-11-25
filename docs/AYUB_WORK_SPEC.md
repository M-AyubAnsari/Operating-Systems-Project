# Technical Specification & Documentation: Target & Instrumentation
**Status:** COMPLETE (100% Validated on Physical Hardware)  
**Lead:** Muhammad Ayub Ansari (Target and Instrumentation Lead)  
**Project:** Persistence of Sensitive Data on Android After App Closure  
**Course:** Applied OS, Cyber Defense and AI Agents  
**Faculty:** Dr. Bilal Wajid, Z. Iqbal, S. Tahir, A. S. Shaikh — Habib University  
**Execution Environment:** 3 Physical Android Devices (300 Runs Executed by Ayub) + Portable Static ARM64 Toolchain  

---

## 1. Role Scope & Completed Deliverables

As the **Target and Instrumentation Lead**, Muhammad Ayub Ansari completed all systems-level engineering, kernel interfacing, and physical device test execution:

1. **Target Test Harness (`src/target_app.c`):**
   * Statically linked pure C application compiled for ARM64 (`target_app_arm64`) using `aarch64-linux-gnu-gcc` with embedded SQLite 3.49.0 single-file amalgamation.
   * Directly interfaces with Android's Bionic C runtime, POSIX filesystem syscalls (`unlink`, `truncate`, `fsync`), POSIX signals (`SIGSEGV`, `SIGKILL`), and Android's logging facility (`<android/log.h>`).
2. **Exit Mode Controller (`src/exit_controller.c`):**
   * Low-level process monitoring through Linux `/proc/<pid>/stat`, screen wake-up via `input keyevent 224/82`, and deterministic trigger of all 5 termination modes (`HOME`, `BACK`, `FORCE_STOP`, `LOW_MEMORY_KILL`, `CRASH`).
3. **Master Experiment Runner (`src/experiment_runner.c`):**
   * Multi-threaded C orchestrator executing the $4 \times 5 \times 5 = 100$ trial matrix per device with microsecond timing ($T_{launch}, T_{setup}, T_{cleanup}$) and cryptographic canary verification.
4. **Physical Hardware Deployment (Zero Emulation):**
   * **Device 1 (Redmi 13):** 100 trials executed via ADB over Wireless (`192.168.18.3:36925`).
   * **Device 2 (Samsung Galaxy A34 5G):** 100 trials executed via direct USB (`RFCW91E6CSY`).
   * **Device 3 (Samsung Galaxy A15):** 100 trials executed via direct USB (`RF8X70EZHEN`).
   * Totaling **300 physical hardware runs** managed and completed by Ayub with a **100.0% positive control pass rate**.

---

## 2. Four Protection Configurations Implemented

| Config ID | Protection Name | Canary Deployment | Logging Policy | Cleanup / Deletion Mechanism |
| :---: | :--- | :--- | :--- | :--- |
| **0** | `CONFIG_BASELINE` | 5 Storage layers (File, SQLite, XML, Cache, SD) | Emitted to Logcat | None (simulates standard unhardened app) |
| **1** | `CONFIG_NO_LOGGING` | 5 Storage layers | Suppressed | None (evaluates log stripping defense) |
| **2** | `CONFIG_PLAIN_CLEANUP` | 5 Storage layers | Suppressed | Standard `unlink()` & SQLite `DELETE FROM` |
| **3** | `CONFIG_SECURE_WIPE` | 5 Storage layers | Suppressed | Cryptographic `0x00` zeroing, `fsync()`, and SQLite `PRAGMA secure_delete = 1; VACUUM;` |

---

## 3. Five Operating System Exit Modes Evaluated

| Exit ID | Mode Name | OS Mechanism & Kernel Behavior | Expected Cleanup Latency |
| :---: | :--- | :--- | :---: |
| **1** | `EXIT_HOME` | Sends `KEYCODE_HOME` (keyevent 3); Android transitions app to Cached Background. | $T_{cleanup} > 0\text{ ms}$ (Graceful) |
| **2** | `EXIT_BACK` | Sends `KEYCODE_BACK` (keyevent 4); invokes graceful activity destruction. | $T_{cleanup} > 0\text{ ms}$ (Graceful) |
| **3** | `EXIT_FORCE_STOP` | Invokes `am force-stop` (sends uncatchable kernel `SIGKILL`). | **$T_{cleanup} = 0.000\text{ ms}$** (Abrupt) |
| **4** | `EXIT_LOW_MEM_KILL` | Simulates Android `lmkd` memory pressure preemption via kernel `kill -9 <pid>`. | **$T_{cleanup} = 0.000\text{ ms}$** (Abrupt) |
| **5** | `EXIT_CRASH` | Triggers immediate unhandled native segmentation fault (`SIGSEGV`). | **$T_{cleanup} = 0.000\text{ ms}$** (Abrupt) |

---

## 4. Systems Depth: Linux Page Cache & Synchronous Flush (`fsync()`) Dynamics

A core systems contribution developed by Ayub is the empirical differentiation between asynchronous POSIX file writes and synchronous storage commits:

```
[Application Space]              [Linux VFS & Kernel]             [Physical Silicon]
   write(fd, buf)  ───────────►  Linux Page Cache (Dirty)  ──────┐
                                                                 │  (Asynchronous pdflush / kswapd)
                                 [Delayed Flush: 5-30s]          ▼
                                 fsync(fd)  ───────────────────► NAND Flash Controller (UFS/eMMC)
```

1. **The Delayed Allocation Window:**
   * When an Android app calls standard `write()` or SQLite `COMMIT`, Linux places the data into the **Page Cache** as dirty memory pages. The kernel does not immediately flush these blocks to physical flash to save battery and reduce wear.
   * In `PLAIN_CLEANUP`, calling `unlink()` simply decrements the directory link count. If the process is killed abruptly, the dirty pages may never be sanitized.
2. **The Temporal Cost of `fsync()`:**
   * In `SECURE_WIPE`, the instrumentation explicitly forces `fsync(fd)` after writing `0x00` buffers. 
   * This forces the storage bus to block until the hardware controller confirms write-to-flash.
   * This explains the measured latency jump from **$5 - 8\text{ ms}$** (Plain) to **$10 - 17\text{ ms}$** (Secure Wipe)—a concrete, measurable demonstration of the I/O cost of operating system security.

---

## 5. Systems Depth: Flash Translation Layer (FTL) & Wear-Leveling

In physical mobile devices, software-level deletion interacts with the underlying silicon architecture in non-intuitive ways:

1. **Logical vs. Physical Block Mapping:**
   * In both **eMMC 5.1** (Redmi 13) and **UFS 2.2/3.1** (Samsung devices), the storage device contains an embedded microcontroller running a **Flash Translation Layer (FTL)**.
   * When `SECURE_WIPE` overwrites logical address $LBA_{100}$ with zeros, the FTL wear-leveling algorithm frequently redirects that write to a *new physical erase block* with lower write counts, merely marking the old physical block as "stale."
2. **The Forensic Implication:**
   * Even after software zeroing, high-end chip-off forensics or JTAG probing can theoretically recover sensitive plaintext fragments from stale NAND erase blocks before hardware garbage collection occurs.
   * True physical security on Android requires hardware-level encryption (UFS Inline Encryption / KeyStore) rather than relying on in-place software wiping.

---

## 6. Handoff Status: Ayub's Work is 100% Complete

All engineering and data collection milestones assigned to Ayub Ansari are fully achieved:
* [x] Target C harness written, statically compiled, and stripped (`target_app_arm64`, `target_app.exe`).
* [x] Exit controller and runner built in pure C (`exit_controller.exe`, `experiment_runner.exe`).
* [x] 100 trials executed on Redmi 13 (`output/experiment_runs_ayub.csv`).
* [x] 100 trials executed on Samsung Galaxy A34 (`output/experiment_runs_samsung1.csv`).
* [x] 100 trials executed on Samsung Galaxy A15 (`output/experiment_runs_samsung2.csv`).
* [x] Page Cache, POSIX signal handling, and FTL architecture documented.

The project is now fully prepared for:
1. **Mahad Khan Khoso:** Run the C forensic carver on sample databases and maintain multi-device compilation.
2. **Muhammad Rashad Wahedna:** Generate statistical figures via `scripts/generate_report_charts.py` and finalize the comparative OS analysis.
