# Team Workflow Reorganization & Multi-Device Experimental Plan

**Course:** Applied OS, Cyber Defense and AI Agents  
**Faculty:** Dr. Bilal Wajid (*"You guys are CS major students. Emulation wont work"*)  
**Team:** Muhammad Ayub Ansari, Mahad Khan Khoso, Muhammad Rashad Wahedna  
**Date:** November 24, 2026  

---

## 1. Hardware Context & Role Alignment

Because **Mahad Khan Khoso** owns an iPhone (iOS) and **Muhammad Ayub Ansari** and **Muhammad Rashad Wahedna** own physical Android phones, the team workflow is optimized to turn this hardware distribution into an academic strength:

| Team Member | Hardware Available | Assigned Responsibilities | Primary Deliverables |
| :--- | :--- | :--- | :--- |
| **Muhammad Ayub Ansari** | Physical Android Phone (Device A) | **Target & Instrumentation Lead**<br>• C Test Harness & Exit Controller<br>• Executes **Batch A** (100 trials on Device A) | • `src/target_app.c`<br>• `src/exit_controller.c`<br>• `src/experiment_runner.c`<br>• `output/experiment_runs_ayub.csv` |
| **Muhammad Rashad Wahedna** | Physical Android Phone (Device B) | **Cross-Validation & Analysis Lead**<br>• Executes **Batch B** (100 trials on Device B)<br>• Statistical charts, heatmaps, and LaTeX tables | • `output/experiment_runs_rashad.csv`<br>• LaTeX plots & figures<br>• Cost trade-off analysis |
| **Mahad Khan Khoso** | iPhone / Analysis Station (PC) | **Data Compilation & Forensics Lead**<br>• Multi-device data compilation & reconciliation<br>• Forensic scanning verification<br>• Report compilation & formatting | • `src/data_compiler.c`<br>• `output/master_compiled_experiment_data.csv`<br>• `output/cross_device_statistical_summary.txt`<br>• Final report draft |

---

## 2. Research Value: Cross-Device Hardware Generalization

Testing across two distinct physical Android devices (e.g., Samsung Galaxy running OneUI/Android 14 vs. Xiaomi Redmi running MIUI/HyperOS/Android 13) elevates the research:

1. **Vendor Kernel & Memory Management Differences:** Proves whether residual memory behavior and process caching after `EXIT_HOME` are universal across Android OEM kernels.
2. **Flash Storage Controller & Filesystem Differences:** Compares SQLite page unlinking and wear-leveling behaviors across different flash storage controllers (UFS vs. eMMC).
3. **Statistical Power:** Increases the dataset from 100 trials to **200 physical device trials** ($100 \times 2$), allowing robust ANOVA and two-sample t-tests in the final report.

---

## 3. Execution Pipeline

```
 [Ayub's Phone A]                        [Rashad's Phone B]
        │                                       │
scripts\run_batch_ayub.bat              scripts\run_batch_rashad.bat
        │                                       │
        ▼                                       ▼
output\experiment_runs_ayub.csv         output\experiment_runs_rashad.csv
        │                                       │
        └───────────────────┬───────────────────┘
                            │
                            ▼
              [Mahad's Analysis Station]
              scripts\compile_data_mahad.bat (data_compiler.exe)
                            │
                            ▼
       output\master_compiled_experiment_data.csv (200 Runs)
       output\cross_device_statistical_summary.txt
                            │
                            ▼
              [Rashad's Figures & Report Section]
```
