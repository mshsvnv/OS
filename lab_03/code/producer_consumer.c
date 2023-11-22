#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 128
#define PROD_AMT 2
#define CONS_AMT 2

#define DEC -1
#define INC 1

#define SB 0
#define SE 1
#define SF 2

struct sembuf prod_start[] = {{SE, DEC, 0}, {SB, DEC, 0}};
struct sembuf prod_stop[]  = {{SB, INC, 0}, {SF, INC, 0}};
struct sembuf cons_start[] = {{SF, DEC, 0}, {SB, DEC, 0}};
struct sembuf cons_stop[]  = {{SB, INC, 0}, {SE, INC, 0}};

int flag = 1;

void sig_handler(int sig_num)
{
    flag = 0;
    printf("Signal %d was caught by %d\n", sig_num, getpid());
}

void producer(char **cur_prod, char *cur_symb, int semid)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 3 + 1);
        int semop_p = semop(semid, prod_start, 2);
        if (semop_p == -1)
        {
            perror("Can't semop.");
            exit(1);
        }
        **cur_prod = *cur_symb;
        printf("Producer %d put \"%c\"\n", getpid(), **cur_prod);
        ++(*cur_prod);
        ++(*cur_symb);
        if (*cur_symb > 'z')
            *cur_symb -= 26;
        int semop_v = semop(semid, prod_stop, 2);
        if (semop_v == -1)
        {
            perror("Can't semop.");
            exit(1);
        }
    }
    exit(0);
}

void consumer(char **cur_cons, int semid)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 3 + 1);
        int semop_p = semop(semid, cons_start, 2);
        if (semop_p == -1)
        {
            perror("Can't semop.");
            exit(1);
        }
        printf("Consumer %d get \"%c\"\n", getpid(), **cur_cons);
        ++(*cur_cons);
        int semop_v = semop(semid, cons_stop, 2);
        if (semop_v == -1)
        {
            perror("Can't semop.");
            exit(1);
        }
    }
    exit(0);
}

int main()
{
    pid_t childpid[PROD_AMT + CONS_AMT], pidw;
    int wstatus;
    int shmid, semid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char **prod, **cons;
    char *symb;

    // установка обработчика сигнала SIGINT
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can't signal.");
        exit(1);
    }
    // разделяемая память
    shmid = shmget(100, BUF_SIZE, IPC_CREAT | perms);
    if (shmid == -1)
    {
        perror("Can't shmget.");
        exit(1);
    }
    char *shm_addr = (char *)shmat(shmid, 0, 0);
    if (shm_addr == (char *)-1)
    {
        perror("Can't shmat.");
        exit(1);
    }
    // семафоры
    semid = semget(100, 3, IPC_CREAT | perms);
    if (semid == -1)
    {
        perror("Can't semget.");
        exit(1);
    }
    int sb = semctl(semid, SB, SETVAL, 1);
    int se = semctl(semid, SE, SETVAL, BUF_SIZE);
    int sf = semctl(semid, SF, SETVAL, 0);
    if (sb == -1 || se == -1 || sf == -1)
    {
        perror("Can't semctl.");
        exit(1);
    }
    // инициализация
    prod = (char **)shm_addr;
    cons = prod + 1;
    symb = (char *)(cons + 1);
    *symb = 'a';
    *prod = symb + 1;
    *cons = symb + 1;
    // производители
    for (int i = 0; i < PROD_AMT; ++i)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            producer(prod, symb, semid);
        }
    }
    // потребители
    for (int i = PROD_AMT; i < PROD_AMT + CONS_AMT; ++i)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            consumer(cons, semid);
        }
    }
    for (int i = 0; i < (PROD_AMT + CONS_AMT); ++i)
    {
        pidw = waitpid(childpid[i], &wstatus, WUNTRACED);
        if (pidw == -1)
        {
            perror("Can't waitpid.");
            exit(1);
        }
        if (WIFEXITED(wstatus))
            printf("Exited %d, status=%d\n", pidw, WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Killed %d by signal %d\n", pidw, WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Stopped %d by signal %d\n", pidw, WSTOPSIG(wstatus));
    }
    if (shmdt(shm_addr) == -1)
    {
        perror("Can't shmdt.");
        exit(1);
    }
    if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    {
        perror("Can't semctl.\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("Can't shmctl.\n");
        exit(1);
    }
    return 0;
}