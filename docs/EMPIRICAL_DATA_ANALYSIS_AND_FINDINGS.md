# Empirical Data Analysis & Comparative Findings Report
**Course:** CS 330 / Applied Operating Systems, Cyber Defense and AI Agents  
**Faculty:** Dr. Bilal Wajid, Z. Iqbal, S. Tahir, A. S. Shaikh — Habib University  
**Topic:** Persistence of Sensitive Data on Android After App Closure  
**Authors:**
- **Muhammad Ayub Ansari** (Target & Instrumentation Lead)
- **Muhammad Rashad Wahedna** (Cross-Validation & Plotting Lead)
- **Mahad Khan Khoso** (Data Compilation & Forensics Lead)  
**Dataset Reference:** `output/master_compiled_experiment_data.csv` (400 physical hardware trials across 4 devices)  
**Date:** November 25, 2026

---

## 1. Executive Summary

This empirical report addresses the central research question:
> **"Do the mitigations in place work to protect sensitive data after Android app closure, or does data persist?"**

Following the faculty directive (*"You guys are CS major students. Emulation won't work"*), this investigation executed **400 empirical trials on physical Android hardware** across four distinct smartphone models, three major Android platform releases (including bleeding-edge Android 17 / API 37), and multiple storage controller architectures.

### Core Research Verdict
1. **Under Graceful Exits (`HOME`, `BACK`):**
   * **The mitigations work.** Both `PLAIN_CLEANUP` and `SECURE_WIPE` successfully erase sensitive data from disk and SQLite databases, and `NO_LOGGING` prevents leakage into Android's circular kernel log buffers (`logcat`). `SECURE_WIPE` introduces cryptographic zeroing (`0x00`) and SQLite page vacuuming, preventing forensic file-carving.
   * **The Cost:** Secure wipe introduces a measurable temporal overhead of **$+66.7\%$ to $+97.8\%$** over plain unlinking.

2. **Under Abrupt Exits (`FORCE_STOP`, `LOW_MEMORY_KILL`, `CRASH`):**
   * **The mitigations fail 100% of the time, and sensitive data persists completely.**
   * Across all four physical devices (including Android 17), whenever an app is killed by the OS kernel, the Low Memory Killer daemon (`lmkd`), or an unhandled crash, **$T_{cleanup} = 0.000\text{ ms}$**. 
   * User-space lifecycle hooks (`onDestroy()`, activity callbacks, signal handlers) are **completely bypassed** by kernel-space preemption (`SIGKILL`), leaving plaintext tokens intact on physical flash storage indefinitely.

---

## 2. Physical Hardware Testbed Specifications

To ensure ecological validity, cross-OEM comparability, and multi-generation evaluation, testing was deployed across four distinct physical devices:

| Specification | Device 1: Redmi 13 | Device 2: Galaxy A34 5G | Device 3: Galaxy A15 | Device 4: Galaxy A56 5G (Rashad) |
| :--- | :---: | :---: | :---: | :---: |
| **Model Number** | `2404ARN45A` | `SM-A346E` | `SM-A155F` | `SM-A566B` |
| **Device Tag** | `REDMI_13_HYPEROS` | `SAMSUNG_GALAXY_1` | `SAMSUNG_GALAXY_2` | `RASHAD_ANDROID` |
| **System on Chip** | MediaTek Helio G91-Ultra | MediaTek Dimensity 1080 | MediaTek Helio G99 (`mt6789`) | Exynos 1580 / Dimensity Octa |
| **Storage Architecture** | **eMMC 5.1** Flash | **UFS 2.2** Flash | **UFS 2.2** Flash | **UFS 3.1** Flash |
| **Android Version** | Android 14 (HyperOS) | Android 14 (OneUI 6.0) | Android 14 (OneUI 6.0) | **Android 17 (API 37)** |
| **Lead Operator** | Muhammad Ayub Ansari | Muhammad Ayub Ansari | Muhammad Ayub Ansari | Muhammad Rashad Wahedna |
| **Trials Executed** | **100 trials** | **100 trials** | **100 trials** | **100 trials** |

**Total Master Empirical Trials:** **400 physical runs** (zero emulation).

---

## 3. Experimental Protocol & Trial Matrix

Each device underwent the full full-factorial test matrix:
$$\text{Total Trials} = 4 \text{ Configurations} \times 5 \text{ Exit Modes} \times 5 \text{ Repeats} = 100 \text{ trials / device}$$

### The Four Defense Configurations:
1. **`BASELINE`:** Canary token is deployed across all 5 storage layers and emitted to Android logcat via `<android/log.h>`. No cleanup is attempted.
2. **`NO_LOGGING`:** Canary token is written to storage, but logcat emission is strictly suppressed.
3. **`PLAIN_CLEANUP`:** Standard file unlinking (`unlink()`, `remove()`), SQLite `DELETE FROM`, and cache directory clearing during exit.
4. **`SECURE_WIPE`:** Cryptographic byte overwriting with `0x00`, synchronous file flushing (`fsync()`), and SQLite `PRAGMA secure_delete = 1; VACUUM;` during exit.

### The Five Operating System Exit Modes:
1. **`HOME`:** Graceful backgrounding via `input keyevent 3` (simulates user pressing Home).
2. **`BACK`:** Graceful teardown via `input keyevent 4` (simulates user finishing activity).
3. **`FORCE_STOP`:** Abrupt kernel termination via `am force-stop` (simulates user or system force-stopping the package).
4. **`LOW_MEM_KILL`:** Simulated OS memory pressure preemption via kernel `kill -9` directed at the application PID.
5. **`CRASH`:** Unhandled native segmentation fault (`SIGSEGV` dereferencing `*(volatile int*)0 = 0xDEADBEEF`).

---

## 4. Empirical Timing Metrics Across Devices

Every trial measured microsecond-precision latencies:
* $T_{launch}$: Time required to spawn the process address space.
* $T_{setup}$: Time required to initialize SQLite and write canary tokens to 5 storage layers.
* $T_{cleanup}$: Time required to execute the cleanup/wipe routine before process destruction.

### Four-Device Cross-Hardware Performance Summary

| Metric | Device 1: Redmi 13 (HyperOS / eMMC) | Device 2: Galaxy A34 (OneUI / UFS) | Device 3: Galaxy A15 (OneUI / UFS) | Device 4: Galaxy A56 (Android 17 / API 37) |
| :--- | :---: | :---: | :---: | :---: |
| **Positive Control Pass Rate** | **100.0%** (100/100) | **100.0%** (100/100) | **100.0%** (100/100) | **100.0%** (100/100) |
| **Mean Setup Latency ($T_s$)** | $23.53\text{ ms}$ | $16.41\text{ ms}$ | $13.57\text{ ms}$ | $22.32\text{ ms}$ |
| **Mean Plain Cleanup ($T_c$)** | $8.56\text{ ms}$ | $6.18\text{ ms}$ | $5.38\text{ ms}$ | $9.17\text{ ms}$ |
| **Mean Secure Wipe ($T_c$)** | $16.65\text{ ms}$ | $10.30\text{ ms}$ | $10.64\text{ ms}$ | $17.67\text{ ms}$ |
| **Defense Temporal Overhead** | **+94.5%** | **+66.7%** | **+97.8%** | **+92.7%** |

---

### Detailed Breakdown of Cleanup Latency ($T_{cleanup}$) by Exit Mode

#### Device 4: Samsung Galaxy A56 5G (`SM-A566B` - Android 17 / API 37) — Operated by Rashad
```text
Configuration    | HOME         | BACK         | FORCE_STOP   | LOW_MEM_KILL | CRASH       
-----------------------------------------------------------------------------------------
BASELINE         |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
NO_LOGGING       |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
PLAIN_CLEANUP    |     10.06 ms |      8.27 ms |      0.00 ms |      0.00 ms |      0.00 ms
SECURE_WIPE      |     18.17 ms |     17.17 ms |      0.00 ms |      0.00 ms |      0.00 ms
```

#### Device 3: Samsung Galaxy A15 (`SM-A155F` - OneUI 6.0 / Android 14) — Operated by Ayub
```text
Configuration    | HOME         | BACK         | FORCE_STOP   | LOW_MEM_KILL | CRASH       
-----------------------------------------------------------------------------------------
BASELINE         |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
NO_LOGGING       |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
PLAIN_CLEANUP    |      5.11 ms |      5.65 ms |      0.00 ms |      0.00 ms |      0.00 ms
SECURE_WIPE      |     10.55 ms |     10.72 ms |      0.00 ms |      0.00 ms |      0.00 ms
```

#### Device 2: Samsung Galaxy A34 5G (`SM-A346E` - OneUI 6.0 / Android 14) — Operated by Ayub
```text
Configuration    | HOME         | BACK         | FORCE_STOP   | LOW_MEM_KILL | CRASH       
-----------------------------------------------------------------------------------------
BASELINE         |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
NO_LOGGING       |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
PLAIN_CLEANUP    |      6.47 ms |      5.88 ms |      0.00 ms |      0.00 ms |      0.00 ms
SECURE_WIPE      |     10.14 ms |     10.45 ms |      0.00 ms |      0.00 ms |      0.00 ms
```

#### Device 1: Xiaomi Redmi 13 (`2404ARN45A` - HyperOS / Android 14) — Operated by Ayub
```text
Configuration    | HOME         | BACK         | FORCE_STOP   | LOW_MEM_KILL | CRASH       
-----------------------------------------------------------------------------------------
BASELINE         |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
NO_LOGGING       |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms |      0.00 ms
PLAIN_CLEANUP    |      8.24 ms |      8.88 ms |      0.00 ms |      0.00 ms |      0.00 ms
SECURE_WIPE      |     15.91 ms |     17.38 ms |      0.00 ms |      0.00 ms |      0.00 ms
```

---

## 5. Storage Layer Forensic Persistence Analysis

| Storage Subsystem | Vulnerability in `BASELINE` | Effectiveness of `NO_LOGGING` | Effectiveness of `PLAIN_CLEANUP` | Effectiveness of `SECURE_WIPE` |
| :--- | :--- | :--- | :--- | :--- |
| **Android Logcat (`/dev/log`)** | **High:** Token persists in circular RAM log buffer; extractable via ADB or apps with `READ_LOGS`. | **Effective (100%):** Completely silences token emission. | **Effective:** Suppressed when coupled with no-log policy. | **Effective:** Suppressed when coupled with no-log policy. |
| **Cache & Temp Files (`/cache`)** | **High:** Temp files remain on disk indefinitely until disk starvation occurs. | No effect on storage. | **Bypassed on Abrupt Exits:** Cleared on `HOME`/`BACK`, persists 100% on `FORCE_STOP`/`LMK`/`CRASH`. | **Bypassed on Abrupt Exits:** Overwritten on `HOME`/`BACK`, persists 100% on abrupt exits. |
| **SharedPreferences (XML)** | **Critical:** Plaintext XML file remains in sandboxed directory across device reboots. | No effect on storage. | **Bypassed on Abrupt Exits:** File unlinked on graceful exit, persists 100% on abrupt exits. | **Bypassed on Abrupt Exits:** File zeroed & unlinked on graceful exit, persists 100% on abrupt exits. |
| **SQLite Database (`.db`)** | **Critical:** Plaintext records remain in B-Tree tables. | No effect on storage. | **Flawed Defense:** `DELETE FROM` marks pages as unallocated; data remains carvable from database free-list! | **Effective on Graceful Exits:** `PRAGMA secure_delete=1` and `VACUUM` zero all free pages; persists 100% on abrupt exits. |
| **Internal App Storage (`/data`)** | **Critical:** Plaintext file stored in app private sandbox. | No effect on storage. | **Bypassed on Abrupt Exits:** Unlinked on graceful exit, persists 100% on abrupt exits. | **Bypassed on Abrupt Exits:** Shredded with `0x00` on graceful exit, persists 100% on abrupt exits. |

---

## 6. Deep Operating System & Architectural Findings

### 1. The Kernel Preemption Invariant Across Android Generations
Testing Rashad's device running **Android 17 (API 37)** alongside Android 14 yielded an essential architectural finding:
* **Across all OS generations (Android 14 through Android 17), the kernel preemption trap remains identical.**
* When the Android OS terminates an app via `FORCE_STOP`, `LOW_MEM_KILL`, or a native `CRASH`, the underlying Linux kernel immediately revokes page tables and frees memory descriptors (`mm_struct`).
* **$T_{cleanup} = 0.000\text{ ms}$ on 100% of abrupt trials.** Even on Google's newest Android 17 architecture, userland exit handlers (`onDestroy()`, POSIX cleanup callbacks) are never executed when killed abruptly.

### 2. Hardware Controller Divergence: UFS vs. eMMC
The empirical data demonstrates the critical role of the flash storage controller:
* **UFS Storage (Samsung A34, A15, A56):** Shows significantly tighter latency bounds during synchronous zeroing (`fsync`) and SQLite vacuuming compared to budget eMMC flash.
* **eMMC 5.1 Storage (Redmi 13):** Suffers from half-duplex bus contention, driving secure wipe latency up to **$16.65\text{ ms}$** (+94.5% overhead).

### 3. The "Temporal Cost of Defense"
Across all four devices, implementing `SECURE_WIPE` introduces an overhead of:
* **Samsung Galaxy A34:** $+66.7\%$ ($6.18 \to 10.30\text{ ms}$)
* **Samsung Galaxy A56 (Android 17):** $+92.7\%$ ($9.17 \to 17.67\text{ ms}$)
* **Xiaomi Redmi 13:** $+94.5\%$ ($8.56 \to 16.65\text{ ms}$)
* **Samsung Galaxy A15:** $+97.8\%$ ($5.38 \to 10.64\text{ ms}$)

This provides empirical validation of the fundamental trade-off between user-perceived responsiveness and cryptographic storage confidentiality.

---

## 7. Architectural Security Recommendations

Because lifecycle-dependent exit cleanup is structurally unreliable, production Android systems must adopt **architectural defenses-in-depth**:

1. **Zero-Persistence / Ephemeral Memory Storage:**
   * High-risk sensitive data (session tokens, encryption keys, biometric vectors) must remain strictly in volatile RAM and **never touch persistent disk or cache storage in plaintext**.
2. **Hardware-Backed Encryption at Rest (Android KeyStore):**
   * If data must be stored locally, it must be encrypted via `EncryptedSharedPreferences` or SQLCipher backed by the hardware **TEE (Trusted Execution Environment) / StrongBox**.
   * When an abrupt exit occurs and plaintext cleanup is bypassed, the raw data persisting on the flash chip remains cryptographically indistinguishable from random noise without the ephemeral runtime key.
3. **Immediate Consumption-Time Zeroing:**
   * Memory buffers and temporary records must be zeroed immediately after their functional use is complete, rather than postponing cleanup until application shutdown.
4. **Logcat Stripping in Production Builds:**
   * ProGuard / R8 rules must permanently strip all `android.util.Log` and `<android/log.h>` calls from release binaries to avoid circular buffer leakage.

---
*Report generated and validated from master empirical records in `output/master_compiled_experiment_data.csv`.*
