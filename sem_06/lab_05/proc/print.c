#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <linux/limits.h>
#include <dirent.h>

#include "print.h"
#include "common.h"

#define BUF_SIZE 1024



void get_cmdline_info(const char *proc, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/cmdline", proc);

    FILE *f = fopen(path, "r");

    char buf[BUF_SIZE + 1];
    int len = fread(buf, 1, BUF_SIZE, f);
    buf[len] = '\0';

    fprintf(out, "CMDLINE\n");
    fprintf(out, "%s\n\n", buf);

    fclose(f);
}

void get_cwd_info(const char *proc, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/cwd", proc);

    char buf[BUF_SIZE + 1];
    int len = readlink(path, buf, BUF_SIZE);
    buf[len] = '\0';

    fprintf(out, "CWD\n");
    fprintf(out, "%s\n\n", buf);
}

void get_environ_info(const char *proc, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/environ", proc);

    FILE *f = fopen(path, "r");
    fprintf(out, "ENVIRON\n");

    char buf[BUF_SIZE + 1];
    int len;

    while ((len = fread(buf, 1, BUF_SIZE, f)) > 0)
    {
        for (int i = 0; i < len; ++i)
            if (buf[i] == 0)
                buf[i] = '\n';

        buf[len] = '\0';
        fprintf(out, "%s", buf);
    }
    fprintf(out, "\n");

    fclose(f);
}

void get_exe_info(const char *proc, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/exe", proc);

    char buf[BUF_SIZE + 1];
    int len = readlink(path, buf, BUF_SIZE);
    buf[len] = '\0';

    fprintf(out, "EXE\n");
    fprintf(out, "%s\n\n", buf);
}

void get_comm_info(const char *proc, FILE *out)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/comm", proc);

    FILE *f = fopen(path, "r");

    char buf[BUF_SIZE + 1];
    int len = fread(buf, 1, BUF_SIZE, f);
    buf[len] = '\0';

    fprintf(out, "COMM\n");
    fprintf(out, "%s\n", buf);

    fclose(f);
}


void get_stat_info(const char *proc, FILE *out) {
    char* headers[] = {"pid", "comm", "state","ppid","pgrp","session","tty_nr","tpgid","flags","minflt","cminflt","majflt","cmajflt","utime","stime","cutime",
		"cstime","priority","nice","num_threads","itrealvalue","starttime","vsize","rss","rsslim","startcode","endcode","startstack","kstkesp","kstkeip","signal","blocked","sigignore",
		"sigcatch","wchan","nswap","cnswap","exit_signal","processor","rt_priority","policy","delayacct_blkio_ticks","guest_time","cguest_time","start_data","end_data","start_brk","arg_start",
		"arg_end","env_start","env_end","exit_code"};
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/stat", proc);

    FILE *f = fopen(path, "r");
    fprintf(out, "STAT\n");

    char buf[BUF_SIZE + 1] = "\0";
    fread(buf, 1, BUF_SIZE, f);

    char *stat_elem = strtok(buf, " ");
    int i = 0;

    while (stat_elem != NULL)
    {
        fprintf(out, headers[i++]);
        fprintf(out, " %s\n", stat_elem);
        stat_elem = strtok(NULL, " ");
    }

    fclose(f);
}

void _ls(FILE *out, const char *dir,int op_a,int op_l)
{
	//Here we will list the directory
	struct dirent *d;
	DIR *dh = opendir(dir);
	if (!dh)
	{
		if (errno = ENOENT)
		{
			//If the directory is not found
			perror("Directory doesn't exist");
		}
		else
		{
			//If the directory is not readable then throw error and exit
			perror("Unable to read directory");
		}
		exit(EXIT_FAILURE);
	}
	//While the next entry is not readable we will print directory files
	while ((d = readdir(dh)) != NULL)
	{
		//If hidden files are found we continue
		if (!op_a && d->d_name[0] == '.')
			continue;
		fprintf(out, "%s  ", d->d_name);
		if(op_l) fprintf(out, "\n");
	}
	if(!op_l)
	fprintf(out, "\n");
}

void get_task_info(const char *proc, FILE *out) {
	fprintf(out, "\n\nTASK\n\n");
	char path[PATH_MAX];
	struct dirent *dir;
	snprintf(path, PATH_MAX, "/proc/%s/task", proc);

	DIR *d = opendir(path);
	if (d) {
	while ((dir = readdir(d)) != NULL) {
		printf("%s\n", dir->d_name);
	}
	closedir(d);
}

}

void print_maps(const char *proc, FILE *out) {
    char *line = NULL; // Инициализируем указатель, чтобы избежать проблем
    int start_addr, end_addr, page_size = 4096;
    size_t line_size = 0; // Инициализируем размер строки
    ssize_t line_length;

    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%s/maps", proc);
    char buf[BUF_SIZE] = {'\0'};
    FILE *file = fopen(pathToOpen, "r");
    if (file == NULL) {
        perror("fopen():");
        return;
    }
    fprintf(out, "\nMAPS:\n");
    int lengthOfRead;
    fprintf(out, "pagesize address perms offset dev inode pathname\n");
    do
    {
        line_length = getline(&line, &line_size, file);
        if (line_length == -1)
        {
            if (!feof(file)) {
                perror("getline():");
            }
            break;
        }
        sscanf(line, "%x-%x", &start_addr, &end_addr);
        fprintf(out, "%d\t%s", (end_addr - start_addr) / page_size, line);
    } while (!feof(file));
    free(line); // Освобождаем память, выделенную под строку
    fclose(file);
}



long int *buf[1000];

int print_pagemap_info(const char *stat, FILE *out) {
	char buffer[BUFSIZ];
	char maps_file[BUFSIZ];
	char pagemap_file[BUFSIZ];
	int maps_fd;
	int offset = 0;
	int pagemap_fd;
	pid_t pid;


	pid = strtoull(stat, NULL, 0);
	snprintf(maps_file, sizeof(maps_file), "/proc/%ju/maps", (uintmax_t)pid);
	snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
	maps_fd = open(maps_file, O_RDONLY);
	if (maps_fd < 0) {
		perror("open maps");
		return EXIT_FAILURE;
	}
	pagemap_fd = open(pagemap_file, O_RDONLY);
	if (pagemap_fd < 0) {
		perror("open pagemap");
		return EXIT_FAILURE;
	}
	fprintf(out, "\nPAGEMAP: \n");
	fprintf(out, "addr pfn soft-dirty file/shared swapped present library\n");
	for (;;) {
		ssize_t length = read(maps_fd, buffer + offset, sizeof buffer - offset);
		if (length <= 0) break;
		length += offset;
		for (size_t i = offset; i < (size_t)length; i++) {
			uintptr_t low = 0, high = 0;
			if (buffer[i] == '\n' && i) {
				const char *lib_name;
				size_t y;
				/* Parse a line from maps. Each line contains a range that contains many pages. */
				{
					size_t x = i - 1;
					while (x && buffer[x] != '\n') x--;
					if (buffer[x] == '\n') x++;
					while (buffer[x] != '-' && x < sizeof buffer) {
						char c = buffer[x++];
						low *= 16;
						if (c >= '0' && c <= '9') {
							low += c - '0';
						} else if (c >= 'a' && c <= 'f') {
							low += c - 'a' + 10;
						} else {
						    break;
						}
					}
					while (buffer[x] != '-' && x < sizeof buffer) x++;
					if (buffer[x] == '-') x++;
					while (buffer[x] != ' ' && x < sizeof buffer) {
						char c = buffer[x++];
						high *= 16;
						if (c >= '0' && c <= '9') {
							high += c - '0';
						} else if (c >= 'a' && c <= 'f') {
							high += c - 'a' + 10;
						} else {
							break;
						}
					}
					lib_name = 0;
					for (int field = 0; field < 4; field++) {
						x++;
						while(buffer[x] != ' ' && x < sizeof buffer) x++;
					}
					while (buffer[x] == ' ' && x < sizeof buffer) x++;
					y = x;
					while (buffer[y] != '\n' && y < sizeof buffer) y++;
					buffer[y] = 0;
					lib_name = buffer + x;
				}
				/* Get info about all pages in this page range with pagemap. */
				{
					PagemapEntry entry;
					for (uintptr_t addr = low; addr < high; addr += sysconf(_SC_PAGE_SIZE)) {
						/* TODO always fails for the last page (vsyscall), why? pread returns 0. */
						if (!pagemap_get_entry(&entry, pagemap_fd, addr)) {
							fprintf(out, "%jx %jx %u %u %u %u %s\n",
								(uintmax_t)addr,
								(uintmax_t)entry.pfn,
								entry.soft_dirty,
								entry.file_page,
								entry.swapped,
								entry.present,
								lib_name
							);
						}
					}
				}
				buffer[y] = '\n';
			}
		}
	}
	close(maps_fd);
	close(pagemap_fd);
	return EXIT_SUCCESS;
}
