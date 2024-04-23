#ifndef _PRINT_H_
#define _PRINT_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void get_cmdline_info(const char *proc, FILE *out);
void get_cwd_info(const char *proc, FILE *out);
void get_environ_info(const char *proc, FILE *out);
void get_exe_info(const char *proc, FILE *out);
void get_stat_info(const char *proc, FILE *out);
void get_comm_info(const char *proc, FILE *out);
void get_task_info(const char *proc, FILE *out);
int print_pagemap_info(const char *proc, FILE *out);
void print_maps(const char *proc, FILE *out);


#endif
