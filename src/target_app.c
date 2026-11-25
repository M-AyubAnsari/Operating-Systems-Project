#include "common.h"
#include <sqlite3.h>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

/* Global paths initialized dynamically from base_dir */
static char path_files[1024];
static char path_db[1024];
static char path_prefs[1024];
static char path_cache[1024];
static char path_external[1024];
static char path_log[1024];

/* Signal handler for simulated crash */
void trigger_fatal_crash(void) {
    fprintf(stderr, "[CRASH_HANDLER] Simulating fatal segmentation fault (SIGSEGV)...\n");
    fflush(stderr);
#if defined(_WIN32)
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif
    /* Terminate immediately with non-zero exit status mimicking fatal crash */
    _exit(139); /* 128 + 11 (SIGSEGV standard POSIX exit status) */
}

/* Helper to securely wipe and unlink a file */
static void secure_overwrite_and_unlink(const char *filepath) {
    FILE *fp = fopen(filepath, "r+b");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        if (size > 0) {
            rewind(fp);
            unsigned char *zero_buf = (unsigned char *)calloc(1, size);
            if (zero_buf) {
                fwrite(zero_buf, 1, size, fp);
                fflush(fp);
                free(zero_buf);
            }
        }
        fclose(fp);
    }
    remove(filepath);
}

/* Setup the 5 storage directories */
static void init_storage_paths(const char *base_dir) {
    char dir_files[512], dir_db[512], dir_prefs[512], dir_cache[512], dir_ext[512], dir_logs[512];
    
    snprintf(dir_files, sizeof(dir_files), "%s/files", base_dir);
    snprintf(dir_db, sizeof(dir_db), "%s/databases", base_dir);
    snprintf(dir_prefs, sizeof(dir_prefs), "%s/shared_prefs", base_dir);
    snprintf(dir_cache, sizeof(dir_cache), "%s/cache", base_dir);
    snprintf(dir_ext, sizeof(dir_ext), "%s/external", base_dir);
    snprintf(dir_logs, sizeof(dir_logs), "%s/logs", base_dir);

    create_dir_if_missing(dir_files);
    create_dir_if_missing(dir_db);
    create_dir_if_missing(dir_prefs);
    create_dir_if_missing(dir_cache);
    create_dir_if_missing(dir_ext);
    create_dir_if_missing(dir_logs);

    snprintf(path_files, sizeof(path_files), "%s/sensitive_vault.txt", dir_files);
    snprintf(path_db, sizeof(path_db), "%s/canary_store.db", dir_db);
    snprintf(path_prefs, sizeof(path_prefs), "%s/canary_prefs.xml", dir_prefs);
    snprintf(path_cache, sizeof(path_cache), "%s/session_cache.tmp", dir_cache);
    snprintf(path_external, sizeof(path_external), "%s/backup_token.dat", dir_ext);
    snprintf(path_log, sizeof(path_log), "%s/logcat_mock.log", dir_logs);
}

/* 1. Internal file storage */
static bool write_internal_file(const char *canary) {
    FILE *fp = fopen(path_files, "w");
    if (!fp) return false;
    fprintf(fp, "AUTH_TOKEN=%s\nISSUED_AT=%lu\nEXPIRY=3600\n", canary, (unsigned long)time(NULL));
    fclose(fp);
    return true;
}

/* 2. SQLite Database storage */
static bool write_sqlite_db(const char *canary) {
    sqlite3 *db = NULL;
    int rc = sqlite3_open(path_db, &db);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }

    /* Enable WAL or journal mode */
    sqlite3_exec(db, "PRAGMA journal_mode = DELETE;", NULL, NULL, NULL);

    const char *sql_create = "CREATE TABLE IF NOT EXISTS secrets ("
                             "id INTEGER PRIMARY KEY, "
                             "category TEXT, "
                             "secret_token TEXT, "
                             "created_at INTEGER);";
    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql_create, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        if (err_msg) sqlite3_free(err_msg);
        sqlite3_close(db);
        return false;
    }

    char sql_insert[512];
    snprintf(sql_insert, sizeof(sql_insert),
             "INSERT OR REPLACE INTO secrets (id, category, secret_token, created_at) "
             "VALUES (1, 'user_session_credential', '%s', %lu);",
             canary, (unsigned long)time(NULL));

    rc = sqlite3_exec(db, sql_insert, NULL, NULL, &err_msg);
    if (err_msg) sqlite3_free(err_msg);
    sqlite3_close(db);
    return (rc == SQLITE_OK);
}

/* 3. Android SharedPreferences XML */
static bool write_shared_prefs(const char *canary) {
    FILE *fp = fopen(path_prefs, "w");
    if (!fp) return false;
    fprintf(fp, "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n"
                "<map>\n"
                "    <string name=\"session_auth_canary\">%s</string>\n"
                "    <boolean name=\"is_authenticated\" value=\"true\" />\n"
                "    <int name=\"user_id\" value=\"4092\" />\n"
                "</map>\n", canary);
    fclose(fp);
    return true;
}

/* 4. Cache directory */
static bool write_cache_file(const char *canary) {
    FILE *fp = fopen(path_cache, "w");
    if (!fp) return false;
    fprintf(fp, "CACHE_SNAPSHOT_KEY=%s;VALID=1;CHECKSUM=849204\n", canary);
    fclose(fp);
    return true;
}

/* 5. External App Storage */
static bool write_external_storage(const char *canary) {
    FILE *fp = fopen(path_external, "w");
    if (!fp) return false;
    fprintf(fp, "EXTERNAL_OFFLINE_BACKUP_CANARY: %s\n", canary);
    fclose(fp);
    return true;
}

/* 6. System Telemetry / Logcat */
static void write_system_log(const char *canary, ProtectionConfig cfg) {
    if (cfg == CONFIG_BASELINE) {
#if defined(__ANDROID__)
        __android_log_print(ANDROID_LOG_DEBUG, "CANARY_HARNESS", "DEBUG: Stored active session canary token: %s", canary);
#endif
        /* In baseline mode, simulate Android logcat output: log.d("CANARY_APP", ...) */
        FILE *fp = fopen(path_log, "a");
        if (fp) {
            fprintf(fp, "[%lu] [CANARY_HARNESS] DEBUG: Stored active session canary token: %s\n",
                    (unsigned long)time(NULL), canary);
            fclose(fp);
        }
    }
}

/* Plain Cleanup routine (Standard delete: references removed, bytes remain in unallocated space) */
static void perform_plain_cleanup(void) {
    /* Standard SQLite delete without VACUUM or secure_delete */
    sqlite3 *db = NULL;
    if (sqlite3_open(path_db, &db) == SQLITE_OK) {
        sqlite3_exec(db, "PRAGMA secure_delete = OFF;", NULL, NULL, NULL);
        sqlite3_exec(db, "DELETE FROM secrets WHERE id=1;", NULL, NULL, NULL);
        sqlite3_close(db);
    }
    /* Standard file unlinking */
    remove(path_files);
    remove(path_prefs);
    remove(path_cache);
    remove(path_external);
}

/* Secure Wipe routine (DoD zero-overwrite before unlink + SQLite secure_delete + VACUUM) */
static void perform_secure_wipe(void) {
    /* Secure SQLite wipe */
    sqlite3 *db = NULL;
    if (sqlite3_open(path_db, &db) == SQLITE_OK) {
        sqlite3_exec(db, "PRAGMA secure_delete = ON;", NULL, NULL, NULL);
        sqlite3_exec(db, "DELETE FROM secrets;", NULL, NULL, NULL);
        sqlite3_exec(db, "VACUUM;", NULL, NULL, NULL);
        sqlite3_close(db);
    }
    /* Overwrite database file directly and delete */
    secure_overwrite_and_unlink(path_db);

    /* Secure overwrite and unlink for other files */
    secure_overwrite_and_unlink(path_files);
    secure_overwrite_and_unlink(path_prefs);
    secure_overwrite_and_unlink(path_cache);
    secure_overwrite_and_unlink(path_external);
}

int main(int argc, char *argv[]) {
    ProtectionConfig cfg = CONFIG_BASELINE;
    ExitMode exit_mode = EXIT_HOME;
    char canary[128] = "CANARY_DEFAULT_TEST_TOKEN_12345";
    char base_dir[256] = "./test_app_data";
    bool verify_active = false;
    bool hold_for_signal = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            cfg = (ProtectionConfig)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--exit-mode") == 0 && i + 1 < argc) {
            exit_mode = (ExitMode)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--canary") == 0 && i + 1 < argc) {
            strncpy(canary, argv[++i], sizeof(canary) - 1);
        } else if (strcmp(argv[i], "--base-dir") == 0 && i + 1 < argc) {
            strncpy(base_dir, argv[++i], sizeof(base_dir) - 1);
        } else if (strcmp(argv[i], "--verify-active") == 0) {
            verify_active = true;
        } else if (strcmp(argv[i], "--hold") == 0) {
            hold_for_signal = true;
        }
    }

    init_storage_paths(base_dir);

    /* Measure setup latency (T_setup) */
    double t_start = get_time_ms();

    bool s1 = write_internal_file(canary);
    bool s2 = write_sqlite_db(canary);
    bool s3 = write_shared_prefs(canary);
    bool s4 = write_cache_file(canary);
    bool s5 = write_external_storage(canary);
    write_system_log(canary, cfg);

    double t_setup = get_time_ms() - t_start;

    /* Positive Control Verification (Verifying canary was populated) */
    bool all_written = s1 && s2 && s3 && s4 && s5;
    if (verify_active) {
        printf("[POSITIVE_CONTROL] Canary populated across vectors: %s\n", all_written ? "PASSED" : "FAILED");
    }

    /* Print JSON telemetry payload for the experiment runner */
    printf("{\"pid\": %ld, \"config\": \"%s\", \"exit_mode\": \"%s\", \"setup_time_ms\": %.3f, \"canary\": \"%s\", \"initialized\": %d}\n",
#if defined(_WIN32)
           (long)GetCurrentProcessId(),
#else
           (long)getpid(),
#endif
           config_to_string(cfg), exit_mode_to_string(exit_mode), t_setup, canary, all_written ? 1 : 0);
    fflush(stdout);

    /* Execute Exit Mode behavior */
    double t_cleanup = 0.0;

    switch (exit_mode) {
        case EXIT_HOME:
            /* Backgrounded: In Android, onStop() is called, but the process stays cached in memory! */
            if (cfg == CONFIG_PLAIN_CLEANUP) {
                double tc_start = get_time_ms();
                perform_plain_cleanup();
                t_cleanup = get_time_ms() - tc_start;
            } else if (cfg == CONFIG_SECURE_WIPE) {
                double tc_start = get_time_ms();
                perform_secure_wipe();
                t_cleanup = get_time_ms() - tc_start;
            }
            printf("{\"status\": \"BACKGROUNDED\", \"cleanup_time_ms\": %.3f}\n", t_cleanup);
            fflush(stdout);
            if (hold_for_signal) {
                /* Simulates Android cached background process waiting in memory */
#if defined(_WIN32)
                Sleep(5000);
#else
                sleep(5);
#endif
            }
            break;

        case EXIT_BACK:
            /* Back button: Activity onDestroy() finishes normally */
            if (cfg == CONFIG_PLAIN_CLEANUP) {
                double tc_start = get_time_ms();
                perform_plain_cleanup();
                t_cleanup = get_time_ms() - tc_start;
            } else if (cfg == CONFIG_SECURE_WIPE) {
                double tc_start = get_time_ms();
                perform_secure_wipe();
                t_cleanup = get_time_ms() - tc_start;
            }
            printf("{\"status\": \"GRACEFUL_SHUTDOWN\", \"cleanup_time_ms\": %.3f}\n", t_cleanup);
            fflush(stdout);
            return 0;

        case EXIT_FORCE_STOP:
            /* Force stop: OS kernel aborts process; lifecycle callbacks NEVER run */
            printf("{\"status\": \"FORCE_STOP_PENDING\", \"cleanup_time_ms\": 0.0}\n");
            fflush(stdout);
            /* Bypasses all cleanups, terminates immediately */
            _exit(0);

        case EXIT_LOW_MEMORY_KILL:
            /* Background LMK kill: app is backgrounded, then killed abruptly by kernel LMK */
            printf("{\"status\": \"LMK_KILLED\", \"cleanup_time_ms\": 0.0}\n");
            fflush(stdout);
            _exit(0);

        case EXIT_CRASH:
            /* Crash: Immediate unhandled SIGSEGV */
            printf("{\"status\": \"CRASH_TRIGGERED\", \"cleanup_time_ms\": 0.0}\n");
            fflush(stdout);
            trigger_fatal_crash();
            break;
    }

    return 0;
}
