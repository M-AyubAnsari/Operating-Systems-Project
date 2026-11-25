#include "common.h"

#define MAX_LINE 2048

typedef struct {
    char operator_name[64];
    char device_tag[64];
    int total_runs;
    int positive_passed;
    double sum_t_launch;
    double sum_t_setup;
    double sum_t_cleanup_plain;
    int count_plain;
    double sum_t_cleanup_secure;
    int count_secure;
    int home_alive_count;
    int home_total_count;
} DeviceSummary;

static void update_summary(DeviceSummary *s, const char *cfg, const char *em,
                           double t_l, double t_s, double t_c, int pos, int alive) {
    s->total_runs++;
    if (pos == 1) s->positive_passed++;
    s->sum_t_launch += t_l;
    s->sum_t_setup += t_s;

    if (strcmp(cfg, "PLAIN_CLEANUP") == 0 && (strcmp(em, "HOME") == 0 || strcmp(em, "BACK") == 0)) {
        s->sum_t_cleanup_plain += t_c;
        s->count_plain++;
    } else if (strcmp(cfg, "SECURE_WIPE") == 0 && (strcmp(em, "HOME") == 0 || strcmp(em, "BACK") == 0)) {
        s->sum_t_cleanup_secure += t_c;
        s->count_secure++;
    }

    if (strcmp(em, "HOME") == 0) {
        s->home_total_count++;
        if (alive == 1) s->home_alive_count++;
    }
}

static int process_single_file(const char *filepath, FILE *f_out, int *global_id, DeviceSummary *sum) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    char line[MAX_LINE];
    bool header = true;
    int count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (header) { header = false; continue; }
        
        int run_id, cfg_id, em_id, rep, pos, alive;
        char op[64] = {0}, dev[64] = {0}, cfg[64] = {0}, em[64] = {0}, token[128] = {0}, stat[64] = {0};
        long pid;
        double tl, ts, tc;

        int n = sscanf(line, "%d,%63[^,],%63[^,],%d,%63[^,],%d,%63[^,],%d,%127[^,],%ld,%d,%lf,%lf,%lf,%d,%63s",
                       &run_id, op, dev, &cfg_id, cfg, &em_id, em, &rep, token, &pid, &alive, &tl, &ts, &tc, &pos, stat);

        if (n >= 15) {
            (*global_id)++;
            count++;
            strncpy(sum->operator_name, op, sizeof(sum->operator_name));
            strncpy(sum->device_tag, dev, sizeof(sum->device_tag));
            update_summary(sum, cfg, em, tl, ts, tc, pos, alive);
            fprintf(f_out, "%d,%s,%s,%d,%s,%d,%s,%d,%s,%ld,%d,%.3f,%.3f,%.3f,%d,%s\n",
                    *global_id, op, dev, cfg_id, cfg, em_id, em, rep, token, pid, alive, tl, ts, tc, pos, stat);
        }
    }
    fclose(fp);
    return count;
}

int main(int argc, char *argv[]) {
    const char *files[] = {
        "output\\experiment_runs_ayub.csv",
        "output\\experiment_runs_samsung1.csv",
        "output\\experiment_runs_samsung2.csv",
        "output\\experiment_runs_rashad.csv"
    };
    int num_files = sizeof(files) / sizeof(files[0]);

    char file_out[256] = "output\\master_compiled_experiment_data.csv";
    char file_summary[256] = "output\\cross_device_statistical_summary.txt";

    printf("====================================================================\n");
    printf("   OPERATING SYSTEMS RESEARCH PROJECT - MULTI-DEVICE DATA COMPILER  \n");
    printf("   Role: Data Compilation & Cross-Device Aggregation                \n");
    printf("   Lead: Mahad Khan Khoso                                           \n");
    printf("====================================================================\n");

    FILE *f_out = fopen(file_out, "w");
    if (!f_out) {
        fprintf(stderr, "[ERROR] Cannot create master output file: %s\n", file_out);
        return 1;
    }

    /* Master CSV Header */
    fprintf(f_out, "global_run_id,operator,device_tag,config_id,config_name,exit_mode_id,"
                   "exit_mode_name,repeat_index,canary_token,pid,process_alive,"
                   "t_launch_ms,t_setup_ms,t_cleanup_ms,positive_control_passed,status\n");

    DeviceSummary summaries[10];
    memset(summaries, 0, sizeof(summaries));
    int valid_devices = 0;
    int global_id = 0;

    for (int i = 0; i < num_files; i++) {
        DeviceSummary s = {0};
        int recs = process_single_file(files[i], f_out, &global_id, &s);
        if (recs > 0) {
            summaries[valid_devices++] = s;
            printf("[SUCCESS] Processed %-36s : %3d records (Device: %s)\n", files[i], recs, s.device_tag);
        }
    }

    fclose(f_out);

    /* Generate Cross-Device Comparison Summary Report */
    FILE *f_sum = fopen(file_summary, "w");
    if (f_sum) {
        fprintf(f_sum, "====================================================================\n");
        fprintf(f_sum, "       CROSS-DEVICE HARDWARE VALIDATION & STATISTICAL SUMMARY       \n");
        fprintf(f_sum, "       Compiled by: Mahad Khan Khoso                                \n");
        fprintf(f_sum, "       Target: Persistence of Sensitive Data on Physical Android    \n");
        fprintf(f_sum, "====================================================================\n\n");

        for (int i = 0; i < valid_devices; i++) {
            DeviceSummary *s = &summaries[i];
            fprintf(f_sum, "DEVICE %d: %s (Operator: %s)\n", i + 1, s->device_tag, s->operator_name);
            fprintf(f_sum, " - Total Trials Executed:       %d\n", s->total_runs);
            fprintf(f_sum, " - Positive Control Pass Rate:  %.1f%% (%d/%d)\n",
                    (double)s->positive_passed * 100.0 / s->total_runs, s->positive_passed, s->total_runs);
            fprintf(f_sum, " - Mean Launch Latency (T_l):   %.2f ms\n", s->sum_t_launch / s->total_runs);
            fprintf(f_sum, " - Mean Setup Latency (T_s):    %.2f ms\n", s->sum_t_setup / s->total_runs);
            if (s->count_plain > 0)
                fprintf(f_sum, " - Mean Plain Cleanup (T_c):    %.2f ms\n", s->sum_t_cleanup_plain / s->count_plain);
            if (s->count_secure > 0)
                fprintf(f_sum, " - Mean Secure Wipe (T_c):      %.2f ms\n", s->sum_t_cleanup_secure / s->count_secure);
            if (s->home_total_count > 0)
                fprintf(f_sum, " - HOME Process Alive Rate:     %.1f%% (%d/%d)\n\n",
                        (double)s->home_alive_count * 100.0 / s->home_total_count, s->home_alive_count, s->home_total_count);
        }

        fprintf(f_sum, "Total Master Compiled Trials: %d\n", global_id);
        fprintf(f_sum, "Cross-device compilation complete and ready for final report formatting.\n");
        fclose(f_sum);
    }

    printf("\n====================================================================\n");
    printf("[COMPLETED] Compilation finished: %d total trials combined across %d devices.\n", global_id, valid_devices);
    printf("[OUTPUT 1] Master Dataset: %s\n", file_out);
    printf("[OUTPUT 2] Statistical Summary: %s\n", file_summary);
    printf("====================================================================\n");

    return 0;
}
