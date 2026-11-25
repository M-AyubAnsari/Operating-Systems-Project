#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_PAGE_SIZE 4096

/*
 * SQLite File Format Header:
 * Offset 0: "SQLite format 3\000" (16 bytes)
 * Offset 16: Page size in bytes (2 bytes, big-endian)
 * Offset 32: First freelist trunk page (4 bytes)
 * Offset 36: Total freelist pages (4 bytes)
 */

static uint16_t read_uint16_be(const unsigned char *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t read_uint32_be(const unsigned char *p) {
    return (uint32_t)((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

void print_hex_dump(const unsigned char *data, size_t len, size_t base_offset) {
    for (size_t i = 0; i < len; i += 16) {
        printf("  0x%08zx: ", base_offset + i);
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len)
                printf("%02X ", data[i + j]);
            else
                printf("   ");
            if (j == 7) printf(" ");
        }
        printf(" |");
        for (size_t j = 0; j < 16 && (i + j) < len; j++) {
            unsigned char c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("|\n");
    }
}

int main(int argc, char *argv[]) {
    printf("====================================================================\n");
    printf("   SQLITE3 RAW BINARY FORENSIC CARVER & FREE-PAGE SCANNER           \n");
    printf("   Role: Storage Forensics & Unallocated Space Recovery             \n");
    printf("   Lead: Mahad Khan Khoso                                           \n");
    printf("====================================================================\n\n");

    if (argc < 2) {
        printf("Usage: %s <sqlite_db_file> [search_token_pattern]\n", argv[0]);
        printf("Default pattern: \"CANARY_\"\n");
        return 1;
    }

    const char *filepath = argv[1];
    const char *pattern = (argc >= 3) ? argv[2] : "CANARY_";
    size_t pat_len = strlen(pattern);

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Cannot open file: %s\n", filepath);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("[FORENSICS] Target Database: %s\n", filepath);
    printf("[FORENSICS] File Size:       %ld bytes\n", file_size);
    printf("[FORENSICS] Search Pattern:  \"%s\"\n\n", pattern);

    if (file_size < 100) {
        printf("[INFO] File too small to be a valid SQLite database.\n");
        fclose(fp);
        return 0;
    }

    unsigned char header[100];
    if (fread(header, 1, 100, fp) != 100) {
        fclose(fp);
        return 1;
    }

    uint16_t raw_page_size = read_uint16_be(header + 16);
    uint32_t page_size = (raw_page_size == 1) ? 65536 : (raw_page_size == 0 ? DEFAULT_PAGE_SIZE : raw_page_size);
    uint32_t freelist_trunk = read_uint32_be(header + 32);
    uint32_t freelist_pages = read_uint32_be(header + 36);

    printf("[SQLITE HEADER METRICS]\n");
    printf("  - Page Size:          %u bytes\n", page_size);
    printf("  - Total Pages:        %ld\n", file_size / page_size);
    printf("  - Freelist Trunk:     Page %u\n", freelist_trunk);
    printf("  - Freelist Pages:     %u\n", freelist_pages);
    printf("--------------------------------------------------------------------\n\n");

    /* Scan whole file for pattern */
    fseek(fp, 0, SEEK_SET);
    unsigned char *buffer = (unsigned char *)malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "[ERROR] Memory allocation failed (%ld bytes)\n", file_size);
        fclose(fp);
        return 1;
    }

    if (fread(buffer, 1, file_size, fp) != (size_t)file_size) {
        fprintf(stderr, "[ERROR] Failed reading entire database into memory\n");
        free(buffer);
        fclose(fp);
        return 1;
    }
    fclose(fp);

    int matches_found = 0;
    for (size_t i = 0; i <= (size_t)file_size - pat_len; i++) {
        if (memcmp(buffer + i, pattern, pat_len) == 0) {
            matches_found++;
            uint32_t page_num = (uint32_t)(i / page_size) + 1;
            uint32_t page_offset = (uint32_t)(i % page_size);

            /* Extract readable token up to space, quote, or null */
            char extracted[128] = {0};
            size_t k = 0;
            while (k < sizeof(extracted) - 1 && (i + k) < (size_t)file_size) {
                unsigned char c = buffer[i + k];
                if (c < 32 || c > 126 || c == '"' || c == '\'' || c == ';') break;
                extracted[k] = (char)c;
                k++;
            }

            printf("[CARVE MATCH #%d] Found sensitive pattern at raw offset 0x%08zX\n", matches_found, i);
            printf("  - Page Number:      Page %u (1-indexed)\n", page_num);
            printf("  - Intra-Page Offset: 0x%04X (%u bytes into page)\n", page_offset, page_offset);
            printf("  - Extracted String: \"%s\"\n", extracted);
            printf("  - Hex Inspection Window:\n");

            size_t dump_start = (i >= 16) ? (i - 16) : 0;
            size_t dump_len = (dump_start + 64 <= (size_t)file_size) ? 64 : ((size_t)file_size - dump_start);
            print_hex_dump(buffer + dump_start, dump_len, dump_start);
            printf("\n");
        }
    }

    printf("====================================================================\n");
    if (matches_found > 0) {
        printf("[FORENSIC VERDICT] VULNERABILITY CONFIRMED: %d lingering token(s) carved!\n", matches_found);
        printf("[EXPLANATION] Although high-level SQLite 'SELECT' queries report empty tables\n");
        printf("              after 'DELETE FROM', the raw string remains intact in unallocated\n");
        printf("              B-Tree leaf pages because plain cleanup does not zero memory.\n");
    } else {
        printf("[FORENSIC VERDICT] CLEAN: Zero matching tokens carved from raw file.\n");
        printf("[EXPLANATION] Database pages were either sanitized via PRAGMA secure_delete=1\n");
        printf("              followed by VACUUM, or the file was securely overwritten.\n");
    }
    printf("====================================================================\n");

    free(buffer);
    return (matches_found > 0) ? 0 : 2;
}
