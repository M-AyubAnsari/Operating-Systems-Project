#include "common.h"

/* Check if a package process is alive on physical Android device over ADB */
bool adb_is_package_alive(const char *package_name, long target_pid) {
    char cmd[512];
    if (target_pid > 0) {
        snprintf(cmd, sizeof(cmd),
                 "adb shell \"[ -d /proc/%ld ] && echo 1 || (pidof target_app || pidof %s)\"",
                 target_pid, package_name);
    } else {
        snprintf(cmd, sizeof(cmd), "adb shell \"pidof target_app || pidof %s\"", package_name);
    }
    FILE *pipe = popen(cmd, "r");
    if (!pipe) return false;
    char buf[128] = {0};
    bool alive = false;
    if (fgets(buf, sizeof(buf), pipe) != NULL) {
        if (atoi(buf) > 0) alive = true;
    }
    pclose(pipe);
    return alive;
}

/* Ensure physical phone screen is awake and unlocked */
void wake_physical_screen(void) {
    system("adb shell input keyevent 224 > nul 2>&1"); /* KEYCODE_WAKEUP */
    system("adb shell input keyevent 82 > nul 2>&1");  /* KEYCODE_MENU (unlock) */
}

/* Execute command helper */
int execute_system_command(const char *cmd) {
    return system(cmd);
}

/* Trigger Android exit mode via ADB or native process control */
int trigger_android_exit(ExitMode em, const char *package_name, long target_pid) {
    char cmd[512];
    int res = 0;

    switch (em) {
        case EXIT_HOME:
            /* Dispatch KEYCODE_HOME (keyevent 3) */
            snprintf(cmd, sizeof(cmd), "adb shell input keyevent 3");
            printf("[EXIT_CONTROLLER] Emulating HOME button press: %s\n", cmd);
            res = execute_system_command(cmd);
            break;

        case EXIT_BACK:
            /* Dispatch KEYCODE_BACK (keyevent 4) */
            snprintf(cmd, sizeof(cmd), "adb shell input keyevent 4");
            printf("[EXIT_CONTROLLER] Emulating BACK button press: %s\n", cmd);
            res = execute_system_command(cmd);
            break;

        case EXIT_FORCE_STOP:
            /* Force-kill via kernel SIGKILL and ActivityManager */
            if (target_pid > 0) {
                snprintf(cmd, sizeof(cmd), "adb shell \"kill -9 %ld 2>/dev/null; pkill -9 target_app 2>/dev/null; am force-stop %s 2>/dev/null\"", target_pid, package_name);
            } else {
                snprintf(cmd, sizeof(cmd), "adb shell \"pkill -9 target_app 2>/dev/null; am force-stop %s 2>/dev/null\"", package_name);
            }
            printf("[EXIT_CONTROLLER] Triggering OS Force-Stop: %s\n", cmd);
            res = execute_system_command(cmd);
            break;

        case EXIT_LOW_MEMORY_KILL:
            /* Step 1: Send to background */
            execute_system_command("adb shell input keyevent 3");
#if defined(_WIN32)
            Sleep(1000);
#else
            sleep(1);
#endif
            /* Step 2: Trigger low memory kill of cached background process */
            if (target_pid > 0) {
                snprintf(cmd, sizeof(cmd), "adb shell \"kill -9 %ld 2>/dev/null; pkill -9 target_app 2>/dev/null; am kill %s 2>/dev/null\"", target_pid, package_name);
            } else {
                snprintf(cmd, sizeof(cmd), "adb shell \"pkill -9 target_app 2>/dev/null; am kill %s 2>/dev/null\"", package_name);
            }
            printf("[EXIT_CONTROLLER] Triggering Low Memory Killer reclamation: %s\n", cmd);
            res = execute_system_command(cmd);
            break;

        case EXIT_CRASH:
            /* Crash signal: Send SIGSEGV directly to PID */
            if (target_pid > 0) {
                snprintf(cmd, sizeof(cmd), "adb shell kill -11 %ld", target_pid);
                printf("[EXIT_CONTROLLER] Injected SIGSEGV (signal 11) to PID %ld: %s\n", target_pid, cmd);
                res = execute_system_command(cmd);
            }
            break;

        default:
            fprintf(stderr, "[EXIT_CONTROLLER] Unknown exit mode: %d\n", em);
            return -1;
    }

    return res;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: exit_controller --mode <1-5> [--pkg <package_name>] [--pid <target_pid>]\n");
        printf("Exit Modes:\n");
        printf("  1: EXIT_HOME (Backgrounded, cached in RAM)\n");
        printf("  2: EXIT_BACK (Graceful lifecycle finish)\n");
        printf("  3: EXIT_FORCE_STOP (OS SIGKILL, no callbacks)\n");
        printf("  4: EXIT_LOW_MEMORY_KILL (Simulated LMK kill)\n");
        printf("  5: EXIT_CRASH (SIGSEGV fatal abort)\n");
        return 1;
    }

    ExitMode em = EXIT_HOME;
    char pkg[128] = "com.osproject.canaryapp";
    long pid = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            em = (ExitMode)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--pkg") == 0 && i + 1 < argc) {
            strncpy(pkg, argv[++i], sizeof(pkg) - 1);
        } else if (strcmp(argv[i], "--pid") == 0 && i + 1 < argc) {
            pid = atol(argv[++i]);
        }
    }

    wake_physical_screen();
    printf("[EXIT_CONTROLLER] Dispatching Exit Mode: %s (%d)\n", exit_mode_to_string(em), em);
    int rc = trigger_android_exit(em, pkg, pid);
    
    /* Query if process is still alive on physical phone or local */
    bool alive = adb_is_package_alive(pkg, pid);
    printf("{\"exit_mode\": \"%s\", \"rc\": %d, \"pid\": %ld, \"process_alive\": %d}\n",
           exit_mode_to_string(em), rc, pid, alive ? 1 : 0);

    return rc;
}
