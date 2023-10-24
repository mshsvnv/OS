#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
    pid_t childpid[2], w;
    int wstatus;
    for (size_t i = 0; i < 2; ++i)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (childpid[i] == 0)
        {
            printf("Child process before pause: PID = %d, PPID = %d, PGRP = %d\n", getpid(), getppid(), getpgrp());
            pause();
            // printf("\nChild process after pause: PID = %d, PPID = %d, PGRP = %d\n", getpid(), getppid(), getpgrp());
            // exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Parent process: PID = %d, PGRP = %d\n", getpid(), getpgrp());
        }
    }
    for (size_t i = 0; i < 2; ++i)
    {    
        w = waitpid(childpid[i], &wstatus, WUNTRACED);
        if (w == -1)
        {
            perror("Can't wait");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wstatus))
            printf("Exited, status=%d\n", WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Killed by signal %d\n", WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Stopped by signal %d\n", WSTOPSIG(wstatus));
    }
    return 0;
}