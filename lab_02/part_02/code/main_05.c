#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
int fl = 0;
void sig_handler(int sig_num) 
{
    fl = 1;
    printf("\nSignal %d was caught by %d\n", sig_num, getpid());
}
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
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can't signal.\n");
        exit(EXIT_FAILURE);
    }
    sleep(2);
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
            if (fl == 1)
            {
                char* message = NULL;
                if (i == 0)
                    message = "iiiiiii\n";
                else    
                    message = "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\n";
                printf("Writen string by %d process: %s\n", getpid(), message);
                close(fd[0]);
                write(fd[1], message, strlen(message));
                close(fd[1]);
            }
            else
            {
                printf("No message!\n");
            }
            exit(EXIT_SUCCESS);
        }
    }
    for (int i = 0; i < 2; ++i) {
        w = waitpid(childpid[i], &wstatus, WUNTRACED);
        if (w == -1)
        {
            perror("Can't waitpid.\n");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wstatus))
            printf("Child ID = %d; Exited, status=%d\n", w, WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Child ID = %d; Killed by signal %d\n", w, WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Child ID = %d; Stopped by signal %d\n", w, WSTOPSIG(wstatus));
        close(fd[1]);
        do
        {
            printf("%c", ch);
        } while (read(fd[0], &ch, 1) > 0);
        close(fd[0]);
    }
    return 0;
}