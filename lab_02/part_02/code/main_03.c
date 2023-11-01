#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void)
{
	int status;
    pid_t children[2];
    char* exe[2] = { "./app_01", "./app_04" };
    for (int i = 0; i < 2; i++)
    {
        if ((children[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        else if (children[i] == 0)
        {
            printf("\nChild %d: PID = %d, PPID = %d, PGRP = %d\n", i + 1, getpid(), getppid(), getpgrp());
            if (execl(exe[i], exe[i], NULL) == -1)
            {
                perror("Can't exec.\n");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
    }
    for (int i = 0; i < 2; i++)
    {
        waitpid(children[i], &status, WUNTRACED);
        if (WIFEXITED(status))
            printf("Child %d finished with code: %d\n", i + 1, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child %d finished with signal %d\n", i + 1, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child %d stopped with signal %d\n", i + 1, WSTOPSIG(status));
        printf("Parent: PID = %d, PGRP = %d, child = %d\n", getpid(), getpgrp(), children[i]);
    }
    return 0;
}
