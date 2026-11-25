# Android Hardware Strategy: Transition to Physical Phone Deployment

**Course:** Applied OS, Cyber Defense and AI Agents  
**Faculty:** Dr. Bilal Wajid, Z. Iqbal, S. Tahir, A. S. Shaikh  
**Team:** Muhammad Ayub Ansari, Mahad Khan Khoso, Muhammad Rashad Wahedna  
**Section Lead (Target & Instrumentation):** Muhammad Ayub Ansari  
**Date:** November 24, 2026  

---

## 1. Faculty Mandate: Physical Device Requirement

Following direct consultation with the course instructor, **Dr. Bilal Wajid**:

> **Student Question:** *"I wanted to ask if using an android emulator is acceptable or do we have to use a physical android phone to gather our data and results?"*  
> **Dr. Bilal Wajid:** ***"You guys are CS major students. Emulation wont work."***

This definitive faculty instruction establishes that all official experimental evaluations and telemetry gathering must execute on a **physical Android phone connected via ADB**.

---

## 2. Technical Challenges & Our Engineering Solutions for Physical Phones

Running low-level OS data remanence experiments on a physical Android device presents several technical constraints. Ayub's instrumentation suite resolves all of them:

| Challenge on Physical Phone | Underlying OS Mechanism | Ayub's C Instrumentation Solution |
| :--- | :--- | :--- |
| **Sandbox Access Without Rooting** | Android uses Linux UIDs (`u0_aXXX`) to isolate `/data/data/<pkg>/`. The `shell` user cannot access other apps' private directories on production non-rooted builds. | **Dual-Path Architecture:**<br>1. **`run-as` Debug Bridge:** Building the target package in debug mode (`android:debuggable="true"`), allowing `adb shell run-as com.osproject.canaryapp` to inspect private sandbox files, SQLite databases, and SharedPreferences without root.<br>2. **Executable Native C Sandbox:** Compiling the native C binary for ARM64 and deploying to `/data/local/tmp/`, which has native POSIX execution rights on retail Android hardware. |
| **Personal Data Safety & Ethics** | Physical phones carry personal accounts, WhatsApp, photos, and system credentials. | **Zero-Touch Isolation:** The test app operates strictly inside its designated package sandbox or `/data/local/tmp/canary_sandbox/`. No external user directories are touched, satisfying Section 14 ethics. |
| **Screen Lock & Sleep Modes** | Phones turn off screens or enter Doze mode, distorting lifecycle timings. | Automation script sends `KEYCODE_WAKEUP` (224) and `KEYCODE_MENU` (82) before each batch and toggles `stay_on_while_plugged_in`. |
| **Variable CPU Frequency (DVFS)** | Thermal throttling and dynamic CPU scaling introduce timing noise to $T_{launch}$ and $T_{setup}$. | Animations disabled (`window_animation_scale 0.0`), repeated trials (5 repeats), and statistical outlier rejection in Rashad's data analysis. |
| **Architecture Difference** | Host machines are x86_64; physical Android phones are **ARM64 (`aarch64-linux-android`)**. | Dual compilation scripts provided: host verification builds (`build.bat`) and cross-compilation builds for ARM64 Android (`build_android_arm64.bat` / Android NDK clang). |

---

## 3. Physical Device Testing Protocol

1. **Hardware Preparation:**
   - Any physical Android device running Android 9.0 to 14.0 (ARM64).
   - Enable **Developer Options** (Tap *Build Number* 7 times in Settings > About Phone).
   - Enable **USB Debugging**.
   - Connect phone to PC via USB cable and authorize computer ("Always allow from this computer").

2. **Verification Command:**
   ```bash
   adb devices
   ```
   *(Must show `<device_serial> device`)*

3. **Running Ayub's Experiment Runner:**
   ```bash
   experiment_runner.exe --repeats 5 --adb --pkg com.osproject.canaryapp --output output/experiment_runs_physical.csv
   ```
