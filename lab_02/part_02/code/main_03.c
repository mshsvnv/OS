#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
    pid_t childpid[2];
    int wstatus;
    const char *const params[] = { "app_01", "app_02" };
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
            printf("Child process before exec: PID = %d, PPID = %d, PGRP = %d\n", getpid(), getppid(), getpgrp());
            int rc = execl(params[i], params[i], 0);
            if (rc == -1)
            {
                perror("Can't exec.\n");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Parent process: PID = %d, PGRP = %d\n", getpid(), getpgrp());
        }
    }
    for (int i = 0; i < 2; i++)
    {
        waitpid(childpid[i], &wstatus, WUNTRACED);
        if (WIFEXITED(wstatus))
            printf("Child %d finished with code: %d\n", i + 1, WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Child %d finished with signal %d\n", i + 1, WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Child %d stopped with signal %d\n", i + 1, WSTOPSIG(wstatus));
        printf("Parent: PID = %d, PGRP = %d, child = %d\n", getpid(), getpgrp(), childpid[i]);
    }
    return 0;
}