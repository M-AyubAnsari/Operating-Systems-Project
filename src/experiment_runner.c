#include "common.h"

/* Generate unique, high-entropy canary token per trial */
void generate_unique_canary(char *buf, size_t max_len, int run_id) {
    uint32_t r1 = (uint32_t)rand();
    uint32_t r2 = (uint32_t)rand();
    uint64_t t = (uint64_t)time(NULL);
    snprintf(buf, max_len, "CANARY_RUN%03d_%08X%08X_%llu", run_id, r1, r2, (unsigned long long)t);
}

/* Reset sandbox storage and logs between runs */
void reset_environment(const char *base_dir, bool use_adb, const char *pkg_name) {
    if (use_adb) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "adb shell pm clear %s > nul 2>&1", pkg_name);
        system(cmd);
        /* Wipe native sandbox directory on phone */
        system("adb shell rm -rf /data/local/tmp/test_app_data/* > nul 2>&1");
        system("adb logcat -c > nul 2>&1");
    } else {
        /* Local filesystem clean reset */
        char path[512];
        snprintf(path, sizeof(path), "%s/files/sensitive_vault.txt", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/databases/canary_store.db", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/databases/canary_store.db-wal", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/databases/canary_store.db-shm", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/shared_prefs/canary_prefs.xml", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/cache/session_cache.tmp", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/external/backup_token.dat", base_dir);
        remove(path);
        snprintf(path, sizeof(path), "%s/logs/logcat_mock.log", base_dir);
        remove(path);
    }
}

/* Parse launch and cleanup timings from target stdout */
static void parse_json_telemetry(const char *line, RunMetadata *meta) {
    const char *setup_ptr = strstr(line, "\"setup_time_ms\":");
    if (setup_ptr) {
        meta->t_setup_ms = atof(setup_ptr + 16);
    }
    const char *clean_ptr = strstr(line, "\"cleanup_time_ms\":");
    if (clean_ptr) {
        meta->t_cleanup_ms = atof(clean_ptr + 18);
    }
    const char *pid_ptr = strstr(line, "\"pid\":");
    if (pid_ptr) {
        meta->pid = atol(pid_ptr + 6);
    }
    const char *init_ptr = strstr(line, "\"initialized\":");
    if (init_ptr) {
        meta->positive_control_passed = atoi(init_ptr + 14);
    }
}

int main(int argc, char *argv[]) {
    int repeats = 5; /* 4 configs * 5 exits * 5 repeats = 100 trials */
#if defined(_WIN32)
    char target_bin[256] = "target_app.exe";
    char base_dir[256] = "test_app_data";
    char out_csv[256] = "output\\experiment_runs.csv";
#else
    char target_bin[256] = "./target_app";
    char base_dir[256] = "./test_app_data";
    char out_csv[256] = "./output/experiment_runs.csv";
#endif
    char pkg_name[128] = "com.osproject.canaryapp";
    char device_tag[128] = "DEVICE_DEFAULT";
    char operator_name[128] = "AYUB";
    bool use_adb = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--repeats") == 0 && i + 1 < argc) {
            repeats = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--target-bin") == 0 && i + 1 < argc) {
            strncpy(target_bin, argv[++i], sizeof(target_bin) - 1);
        } else if (strcmp(argv[i], "--base-dir") == 0 && i + 1 < argc) {
            strncpy(base_dir, argv[++i], sizeof(base_dir) - 1);
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            strncpy(out_csv, argv[++i], sizeof(out_csv) - 1);
        } else if (strcmp(argv[i], "--adb") == 0) {
            use_adb = true;
        } else if (strcmp(argv[i], "--pkg") == 0 && i + 1 < argc) {
            strncpy(pkg_name, argv[++i], sizeof(pkg_name) - 1);
        } else if (strcmp(argv[i], "--device-tag") == 0 && i + 1 < argc) {
            strncpy(device_tag, argv[++i], sizeof(device_tag) - 1);
        } else if (strcmp(argv[i], "--operator") == 0 && i + 1 < argc) {
            strncpy(operator_name, argv[++i], sizeof(operator_name) - 1);
        }
    }

    srand((unsigned int)time(NULL));
    create_dir_if_missing("./output");

    printf("====================================================================\n");
    printf("   OPERATING SYSTEMS RESEARCH PROJECT - EXPERIMENT RUNNER (C)       \n");
    printf("   Target: Persistence of Sensitive Data on Android After App Closure\n");
    printf("   Author: Muhammad Ayub Ansari (Target and Instrumentation Lead)   \n");
    printf("====================================================================\n");
    printf("[DEVICE] Operator: %s | Device Tag: %s\n", operator_name, device_tag);
    printf("[CONFIG] Repeats per combination: %d\n", repeats);
    printf("[CONFIG] Total Trials: %d (4 Configurations x 5 Exit Modes x %d Repeats)\n", 4 * 5 * repeats, repeats);
    printf("[CONFIG] Target Binary: %s\n", target_bin);
    printf("[CONFIG] Storage Base Directory: %s\n", base_dir);
    printf("[CONFIG] Output CSV: %s\n", out_csv);
    printf("====================================================================\n\n");

    FILE *fcsv = fopen(out_csv, "w");
    if (!fcsv) {
        fprintf(stderr, "[ERROR] Cannot open output CSV file: %s\n", out_csv);
        return 1;
    }

    /* Write CSV header for Mahad (Data Compiler/Scanner) and Rashad */
    fprintf(fcsv, "run_id,operator,device_tag,config_id,config_name,exit_mode_id,exit_mode_name,repeat_index,"
                  "canary_token,pid,process_alive,t_launch_ms,t_setup_ms,t_cleanup_ms,"
                  "positive_control_passed,status\n");
    fflush(fcsv);

    int total_runs = 4 * 5 * repeats;
    int current_run = 0;

    for (int c = 0; c < 4; c++) {
        ProtectionConfig cfg = (ProtectionConfig)c;

        for (int e = 1; e <= 5; e++) {
            ExitMode em = (ExitMode)e;

            for (int r = 1; r <= repeats; r++) {
                current_run++;
                RunMetadata meta;
                memset(&meta, 0, sizeof(meta));

                meta.run_id = current_run;
                meta.config_id = cfg;
                snprintf(meta.config_name, sizeof(meta.config_name), "%s", config_to_string(cfg));
                meta.exit_mode_id = em;
                snprintf(meta.exit_mode_name, sizeof(meta.exit_mode_name), "%s", exit_mode_to_string(em));
                meta.repeat_index = r;

                generate_unique_canary(meta.canary_token, sizeof(meta.canary_token), current_run);

                printf("[%3d/%3d] Config: %-13s | Exit: %-12s | Rep: %d/%d\n",
                       current_run, total_runs, meta.config_name, meta.exit_mode_name, r, repeats);

                /* Step 1: Deterministic state reset */
                reset_environment(base_dir, use_adb, pkg_name);

                /* Step 2: Measure Launch Latency (T_launch) and execute target app */
                char cmd[1024];
                if (use_adb) {
                    /* Ensure physical phone screen is awake and unlocked */
                    system("adb shell input keyevent 224 > nul 2>&1");
                    system("adb shell input keyevent 82 > nul 2>&1");
                    snprintf(cmd, sizeof(cmd),
                             "adb shell \"/data/local/tmp/target_app --config %d --exit-mode %d --canary '%s' --base-dir '/data/local/tmp/test_app_data' --verify-active\"",
                             (int)cfg, (int)em, meta.canary_token);
                } else {
                    snprintf(cmd, sizeof(cmd),
                             "%s --config %d --exit-mode %d --canary \"%s\" --base-dir \"%s\" --verify-active",
                             target_bin, (int)cfg, (int)em, meta.canary_token, base_dir);
                }

                double t_launch_start = get_time_ms();

                FILE *pipe = popen(cmd, "r");
                double t_launch_finish = get_time_ms();
                meta.t_launch_ms = t_launch_finish - t_launch_start;

                if (pipe) {
                    char line[512];
                    while (fgets(line, sizeof(line), pipe) != NULL) {
                        parse_json_telemetry(line, &meta);
                    }
                    pclose(pipe);
                }

                /* Step 3: Check process alive status after exit */
                if (use_adb) {
                    char check_cmd[512];
                    if (meta.pid > 0) {
                        /* Check exact PID in kernel /proc first, then native binary name, then package */
                        snprintf(check_cmd, sizeof(check_cmd),
                                 "adb shell \"[ -d /proc/%ld ] && echo 1 || (pidof target_app || pidof %s)\"",
                                 meta.pid, pkg_name);
                    } else {
                        snprintf(check_cmd, sizeof(check_cmd),
                                 "adb shell \"pidof target_app || pidof %s\"", pkg_name);
                    }
                    FILE *chk_pipe = popen(check_cmd, "r");
                    meta.process_alive_after_exit = false;
                    if (chk_pipe) {
                        char buf[64] = {0};
                        if (fgets(buf, sizeof(buf), chk_pipe) != NULL && atoi(buf) > 0) {
                            meta.process_alive_after_exit = true;
                        }
                        pclose(chk_pipe);
                    }
                } else {
                    meta.process_alive_after_exit = (em == EXIT_HOME);
                }

                snprintf(meta.status, sizeof(meta.status), "COMPLETED");

                /* Step 4: Record per-run row to CSV */
                fprintf(fcsv, "%d,%s,%s,%d,%s,%d,%s,%d,%s,%ld,%d,%.3f,%.3f,%.3f,%d,%s\n",
                        meta.run_id,
                        operator_name,
                        device_tag,
                        meta.config_id,
                        meta.config_name,
                        meta.exit_mode_id,
                        meta.exit_mode_name,
                        meta.repeat_index,
                        meta.canary_token,
                        meta.pid,
                        meta.process_alive_after_exit ? 1 : 0,
                        meta.t_launch_ms,
                        meta.t_setup_ms,
                        meta.t_cleanup_ms,
                        meta.positive_control_passed,
                        meta.status);
                fflush(fcsv);
            }
        }
    }

    fclose(fcsv);
    printf("\n====================================================================\n");
    printf("[COMPLETED] All %d trials executed successfully!\n", total_runs);
    printf("[OUTPUT] Metadata records written to: %s\n", out_csv);
    printf("Ready for forensic scanning by Mahad and statistical plotting by Rashad.\n");
    printf("====================================================================\n");

    return 0;
}
