#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 64000
#define PATH_MAX 64000

void print_page(uint64_t address, uint64_t data, FILE *out) {
    fprintf(out, "0x%-16lx : %-16lx %-10ld %-10ld %-10ld %-10ld\n", address,
            data & (((uint64_t)1 << 55) - 1), (data >> 55) & 1,
            (data >> 61) & 1, (data >> 62) & 1, (data >> 63) & 1);
}

void get_pagemap_info(const char *proc, FILE *out) {
    fprintf(out, "PAGEMAP\n");
    fprintf(out, " addr : pfn soft-dirty file/shared swapped present\n");

    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/maps", proc);
    FILE *maps = fopen(path, "r");
    printf("Path: %s\n", path);
    if (maps == NULL)
        printf("GG");

    snprintf(path, PATH_MAX, "/proc/%s/pagemap", proc);
    int pm_fd = open(path, O_RDONLY);

    char buf[BUF_SIZE + 1] = "\0";
    int len;

    while ((len = fread(buf, 1, BUF_SIZE, maps)) > 0) {
        for (int i = 0; i < len; ++i)
            if (buf[i] == 0)
                buf[i] = '\n';
        buf[len] = '\0';

        char *save_row;
        char *row = strtok_r(buf, "\n", &save_row);

        while (row) {
            char *addresses = strtok(row, " ");
            char *start_str, *end_str;

            if ((start_str = strtok(addresses, "-")) && (end_str = strtok(NULL, "-"))) {
                uint64_t start = strtoul(start_str, NULL, 16);
                uint64_t end = strtoul(end_str, NULL, 16);

                for (uint64_t i = start; i < end; i += sysconf(_SC_PAGE_SIZE)) {
                    uint64_t offset;
                    uint64_t index = i / sysconf(_SC_PAGE_SIZE) * sizeof(offset);

                    pread(pm_fd, &offset, sizeof(offset), index);
                    print_page(i, offset, out);
                }
            }
            row = strtok_r(NULL, "\n", &save_row);
        }
    }

    fclose(maps);
    close(pm_fd);
}


int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s pid\n", argv[0]);
		return EXIT_FAILURE;
	}

    FILE *out_file = fopen(argv[2], "w");
    get_pagemap_info(argv[1], out_file);

    fclose(out_file);
}