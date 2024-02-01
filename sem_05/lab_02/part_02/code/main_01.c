#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
    pid_t children[2];
    for (size_t i = 0; i < 2; i++)
    {
        children[i] = fork();
        if (children[i] == -1)
        {
            perror("Can't fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (children[i] == 0)
        {
            printf("Child process before sleep: PID = %d, PPID = %d, PGRP = %d\n", getpid(), getppid(), getpgrp());
            sleep(2);
            printf("\nChild process after sleep: PID = %d, PPID = %d, PGRP = %d\n", getpid(), getppid(), getpgrp());
            return 0;
        }
        else
        {
            printf("Parent process: PID = %d, ChildID = %d, PGRP = %d\n", getpid(), children[i], getpgrp());
        }
    }
    return 0;
}