#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define mkdir_portable(path) _mkdir(path)
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#define mkdir_portable(path) mkdir(path, 0777)
#endif

/* Protection Configurations */
typedef enum {
    CONFIG_BASELINE       = 0, /* No protection, canary written to all 5 locations & logcat */
    CONFIG_NO_LOGGING     = 1, /* Canary written to 5 storage locations, logging suppressed */
    CONFIG_PLAIN_CLEANUP  = 2, /* Ordinary delete on stop (removes references; raw bytes linger in unallocated DB/FS blocks) */
    CONFIG_SECURE_WIPE    = 3  /* Cryptographic zero-fill overwrite before unlink, SQLite secure_delete=ON + VACUUM, no logging */
} ProtectionConfig;

/* Exit Modes */
typedef enum {
    EXIT_HOME             = 1, /* Backgrounded via Home button (process stays alive in cached state) */
    EXIT_BACK             = 2, /* Exited via Back button (standard graceful lifecycle shutdown) */
    EXIT_FORCE_STOP       = 3, /* Abrupt OS kill (am force-stop / SIGKILL, bypassing cleanup callbacks) */
    EXIT_LOW_MEMORY_KILL  = 4, /* Simulated Low-Memory Killer (backgrounded, then abruptly reclaimed by LMKD) */
    EXIT_CRASH            = 5  /* Unhandled SIGSEGV crash (app aborts immediately, leaving all open state intact) */
} ExitMode;

/* Target Persistence Storage Vectors */
#define NUM_STORAGE_VECTORS 5
typedef enum {
    VECTOR_INTERNAL_FILE  = 0, /* /data/data/<pkg>/files/sensitive_vault.txt */
    VECTOR_SQLITE_DB      = 1, /* /data/data/<pkg>/databases/canary_store.db */
    VECTOR_SHARED_PREFS   = 2, /* /data/data/<pkg>/shared_prefs/canary_prefs.xml */
    VECTOR_CACHE_FILE     = 3, /* /data/data/<pkg>/cache/session_cache.tmp */
    VECTOR_EXTERNAL_STORE = 4  /* /sdcard/Android/data/<pkg>/files/backup_token.dat */
} StorageVector;

/* String helpers for configurations and exit modes */
static inline const char* config_to_string(ProtectionConfig cfg) {
    switch (cfg) {
        case CONFIG_BASELINE:      return "BASELINE";
        case CONFIG_NO_LOGGING:    return "NO_LOGGING";
        case CONFIG_PLAIN_CLEANUP: return "PLAIN_CLEANUP";
        case CONFIG_SECURE_WIPE:   return "SECURE_WIPE";
        default:                   return "UNKNOWN_CONFIG";
    }
}

static inline const char* exit_mode_to_string(ExitMode em) {
    switch (em) {
        case EXIT_HOME:            return "HOME";
        case EXIT_BACK:            return "BACK";
        case EXIT_FORCE_STOP:      return "FORCE_STOP";
        case EXIT_LOW_MEMORY_KILL: return "LOW_MEM_KILL";
        case EXIT_CRASH:           return "CRASH";
        default:                   return "UNKNOWN_EXIT";
    }
}

/* Run Metadata Structure */
typedef struct {
    int run_id;
    int config_id;
    char config_name[32];
    int exit_mode_id;
    char exit_mode_name[32];
    int repeat_index;
    char canary_token[128];
    long pid;
    bool process_alive_after_exit;
    double t_launch_ms;
    double t_setup_ms;
    double t_cleanup_ms;
    int positive_control_passed;
    char status[32];
} RunMetadata;

/* High-resolution timestamp helper in milliseconds */
static inline double get_time_ms(void) {
#if defined(_WIN32)
    static LARGE_INTEGER freq;
    static int init = 0;
    if (!init) {
        QueryPerformanceFrequency(&freq);
        init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0);
#endif
}

/* Recursive directory creation helper */
static inline void create_dir_if_missing(const char *dir) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", dir);
    size_t len = strlen(tmp);
    if (len == 0) return;
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') tmp[len - 1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            mkdir_portable(tmp);
#if defined(_WIN32)
            *p = '\\';
#else
            *p = '/';
#endif
        }
    }
    mkdir_portable(tmp);
}

#endif /* COMMON_H */
