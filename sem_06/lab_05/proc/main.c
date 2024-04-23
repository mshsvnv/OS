#include "print.h"

int main(int argc, char *argv[]) {
    const char *pid = argv[1];
    char *pid_copy = strdup(pid);
    FILE *file = fopen("output.txt", "w");
    FILE *file_pm = fopen("output_pagemap.txt", "w");
    if (!file) {
        perror("fopen() error");
        exit(1);
    }
    
    print_maps(pid, stdout);
    get_task_info(pid, stdout);
    
    get_cmdline_info(pid, file);
    get_cwd_info(pid, file);
    get_environ_info(pid, file);
    get_stat_info(pid, file);
    get_exe_info(pid, file);
    get_comm_info(pid, file);

    print_pagemap_info(pid, file_pm);
    fclose(file);
    fclose(file_pm);
    return 0;
}
