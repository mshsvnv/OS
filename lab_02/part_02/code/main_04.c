#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(void)
{
    pid_t childpid[2], w;
    int wstatus;
    char ch;
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("Can't pipe.\n");
        exit(EXIT_FAILURE);
    }
    char buf[256] = { 0 };
    const char* messages[] = { "iiiiiii\n", "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\n" };
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
            printf("Child process: PID = %d, PPID = %d, PGRP = %d\n", getpid(), getppid(), getpgrp());
            close(fd[0]);
            write(fd[1], messages[i], strlen(messages[i]));
            close(fd[1]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Parent process: PID = %d, PGRP = %d\n", getpid(), getpgrp());
        }
    }
    for (int i = 0; i < 2; ++i) {
        w = waitpid(childpid[i], &wstatus, WUNTRACED);
        if (w == -1)
        {
            perror("Can't waitpid.");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wstatus))
            printf("Exited, status=%d\n", WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Killed by signal %d\n", WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Stopped by signal %d\n", WSTOPSIG(wstatus));
        close(fd[1]);
        while (read(fd[0], &ch, 1) > 0)
            printf("%c", ch);
        close(fd[0]);
    }
    printf("Repeat pipe:\n");
	close(fd[1]);
	read(fd[0], buf, strlen(buf));
	printf("Message #%d: '%s'\n\n", 3, buf);
    return 0;
}