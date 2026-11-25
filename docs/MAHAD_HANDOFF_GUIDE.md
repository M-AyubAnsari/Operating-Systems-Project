# Step-by-Step Lead Guide & Deliverables for Mahad Khan Khoso
**Role:** Storage Forensics & Data Compilation Lead  
**Team Member:** Mahad Khan Khoso  
**Project:** Persistence of Sensitive Data on Android After App Closure  
**Course:** Applied OS, Cyber Defense and AI Agents (Habib University)  
**Faculty:** Dr. Bilal Wajid, Z. Iqbal, S. Tahir, A. S. Shaikh  

---

## 1. Mahad's Core Role & Technical Ownership

As the **Storage Forensics & Data Compilation Lead**, you own two critical systems components:
1. **Low-Level Storage Forensics:** Demonstrating how data lingers in storage layers and proving that SQLite `DELETE FROM` is a forensic illusion using the custom pure C carver (`forensic_carver.exe`).
2. **Multi-Device Data Pipeline:** Aggregating, validating, and compiling the master dataset across all 4 physical devices (400 trials) using `data_compiler.exe`.
3. **Report Authorship:** Writing the dedicated sections on Storage Forensics, B-Tree Page Mechanics, and Multi-Device Aggregation.

---

## 2. Tool 1: SQLite Free-Page Binary Carver (`src/forensic_carver.c`)

We have built and compiled a standalone forensic tool in pure C for your role: `forensic_carver.exe`.

### Why This Tool Matters for Your Grade:
When developers execute `DELETE FROM table;` in SQLite (our `PLAIN_CLEANUP` configuration), standard users assume the data is gone because high-level SQL queries (`SELECT *`) return 0 rows. 
**Your tool proves that the data remains intact in unallocated B-Tree leaf pages on disk.**

### How to Run the Carver:
Open a command prompt in the project root and run:
```cmd
forensic_carver.exe <path_to_db_file> [optional_search_pattern]
```
*(Default search pattern is `"CANARY_"`)*.

### What the Output Demonstrates:
```text
====================================================================
   SQLITE3 RAW BINARY FORENSIC CARVER & FREE-PAGE SCANNER           
   Role: Storage Forensics & Unallocated Space Recovery             
   Lead: Mahad Khan Khoso                                           
====================================================================
[FORENSICS] Target Database: sample_database.db
[FORENSICS] File Size:       8192 bytes
[FORENSICS] Search Pattern:  "CANARY_"

[SQLITE HEADER METRICS]
  - Page Size:          4096 bytes
  - Total Pages:        2
  - Freelist Trunk:     Page 0
  - Freelist Pages:     0
--------------------------------------------------------------------
[CARVE MATCH #1] Found sensitive pattern at raw offset 0x00001FD7
  - Page Number:      Page 2 (1-indexed)
  - Intra-Page Offset: 0x0FD7 (4055 bytes into page)
  - Extracted String: "CANARY_SECRET_TOKEN_12345"
  - Hex Inspection Window:
  0x00001fd7: 43 41 4E 41 52 59 5F 53  45 43 52 45 54 5F 54 4F  |CANARY_SECRET_TO|
====================================================================
[FORENSIC VERDICT] VULNERABILITY CONFIRMED: Lingering token(s) carved!
[EXPLANATION] Although high-level SQLite 'SELECT' queries report empty tables
              after 'DELETE FROM', the raw string remains intact in unallocated
              B-Tree leaf pages because plain cleanup does not zero memory.
====================================================================
```

### Presentation Talking Point for Mahad:
> *"During our defense, I can show Dr. Bilal that a standard SQL query returns an empty table, but my low-level binary scanner proves the raw plaintext token is still resident on physical flash because SQLite's default engine only unlinks the B-Tree record without zeroing the page."*

---

## 3. Tool 2: Multi-Device Master Data Compiler (`src/data_compiler.c`)

You maintain and execute the multi-device data aggregation pipeline in pure C: `data_compiler.exe`.

### Datasets Processed:
The compiler automatically merges all four 100-trial batches from physical silicon:
1. `output/experiment_runs_ayub.csv` (Device 1: Xiaomi Redmi 13 / HyperOS / eMMC 5.1)
2. `output/experiment_runs_samsung1.csv` (Device 2: Samsung Galaxy A34 5G / OneUI 6.0 / UFS 2.2)
3. `output/experiment_runs_samsung2.csv` (Device 3: Samsung Galaxy A15 / OneUI 6.0 / UFS 2.2)
4. `output/experiment_runs_rashad.csv` (Device 4: Samsung Galaxy A56 5G / Android 17 / API 37)

### How to Run:
Double click `scripts\compile_data_mahad.bat` or run:
```cmd
data_compiler.exe
```
This produces:
* **`output/master_compiled_experiment_data.csv`:** Unified 400-trial dataset with sequential `global_run_id` (401 lines).
* **`output/cross_device_statistical_summary.txt`:** Statistical digest comparing latencies and pass rates.

---

## 4. Mahad's Report Writing Checklist

When formatting the final submission paper matching Dr. Bilal Wajid's sample format (`Project - Part IV - Sample (B. Wajid).pdf`), Mahad is responsible for authoring:

* [ ] **Section 3.1: Storage Architecture & Abstraction Layers**
  * Detail how Android manages private files (`/data/data/<pkg>/files`), SQLite databases (`/databases`), SharedPreferences (`/shared_prefs`), and external storage (`/sdcard`).
* [ ] **Section 3.2: SQLite B-Tree Free-List Mechanics**
  * Explain why `DELETE FROM` fails to protect sensitive data at rest.
  * Contrast `PLAIN_CLEANUP` with `PRAGMA secure_delete = 1;` and `VACUUM;`.
  * Embed screenshots/hex dumps from `forensic_carver.exe`.
* [ ] **Section 4.1: Data Compilation & Reconciliation Protocol**
  * Describe how `data_compiler.c` ingested 400 trials across 4 devices, enforced positive control verification, and verified zero cross-run token leakage.
* [ ] **Slide Preparation:**
  * Prepare 2 presentation slides explaining the SQLite Free-Page Carving vulnerability and showing the live C terminal demo.
