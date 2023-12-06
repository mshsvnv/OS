#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define ACTIVE_READERS 0
#define ACTIVE_WRITER  1
#define WRITE_QUEUE    2
#define READ_QUEUE     3
#define BIN_WRITING    4

#define CNT_WRITER 3
#define CNT_READER 5

#define P -1
#define V  1

struct sembuf start_read[] = 
{
    {READ_QUEUE, V, 0},           
    {BIN_WRITING, 0, 0},         
    {WRITE_QUEUE, 0, 0},         
    {ACTIVE_READERS, V, 0},      
    {READ_QUEUE, P, 0}           
};

struct sembuf stop_read[] = 
{
    {ACTIVE_READERS, P, 0}       
};

struct sembuf start_write[] = 
{
    {WRITE_QUEUE, V, 0},          
    {ACTIVE_READERS, 0, 0},       
    {BIN_WRITING, 0, 0},          
    {ACTIVE_WRITER, P, 0},        
    {BIN_WRITING, V, 0},          
    {WRITE_QUEUE, P, 0}           
};

struct sembuf stop_write[] = 
{
    {BIN_WRITING, P, 0},           
    {ACTIVE_WRITER, V, 0}          
};

int flag = 1;

void sig_handler(int sig_num)
{
    if (getppid() == getpgrp())
    {
        flag = 0;
  
        printf("\nProcess %d catch signal %d\n", getpid(), sig_num);
    }
}

void writer(int *shared_num, int semid)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 3);
        int semop_p = semop(semid, start_write, 6);
        if (semop_p == -1)
        {
            perror("Can't semop (start_write)");
            exit(1);
        }
        ++(*shared_num);
        printf("Writer %d increment shared_num = \'%d\'\n", getpid(), *shared_num);
        int semop_v = semop(semid, stop_write, 2);
        if (semop_v == -1)
        {
            perror("Can't semop (stop_write)");
            exit(1);
        }
    }
    exit(0);
}

void reader(int *shared_num, int semid)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 3);
        int semop_p = semop(semid, start_read, 5);
        if (semop_p == -1)
        {
            perror("Can't semop (start_read)");
            exit(1);
        }
        printf("Reader %d get shared_num = \'%d\'\n", getpid(), *shared_num);
        int semop_v = semop(semid, stop_read, 1);
        if (semop_v == -1)
        {
            perror("Can't semop (stop_read)");
            exit(1);
        }
    }
    exit(0);
}

int main(void)
{
    pid_t childpid[CNT_WRITER + CNT_READER], pidw;
    int wstatus;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP;
    int semid, shmid;
    int *shared_num;

    // установка обработчика сигнала SIGINT
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can't signal");
        exit(1);
    }

    // разделяемая память
    shmid = shmget(100, sizeof(int), IPC_CREAT | perms);
    if (shmid == -1)
    {
        perror("Can't shmid");
        exit(1);
    }

    shared_num = (int *)shmat(shmid, 0, perms);
    if (shared_num == (int *)-1)
    {
        perror("Can't shmat");
        exit(1); 
    }

    // семафоры
    semid = semget(100, 5, IPC_CREAT | perms);
    if (semid == -1)
    {
        perror("Can't semget");
        exit(1);
    }

    *shared_num = 0;
    if (semctl(semid, ACTIVE_READERS, SETVAL, 0) == -1)
    {
        perror("Can't semctl (ACTIVE_READERS)");
        exit(1);
    }
    if (semctl(semid, ACTIVE_WRITER, SETVAL, 1) == -1)
    {
        perror("Can't semctl (ACTIVE_WRITER)");
        exit(1);
    }
    if (semctl(semid, READ_QUEUE, SETVAL, 0) == -1)
    {
        perror("Can't semctl (READ_QUEUE)");
        exit(1);
    }
    if (semctl(semid, WRITE_QUEUE, SETVAL, 0) == -1)
    {
        perror("Can't semctl (WRITE_QUEUE)");
        exit(1);
    }
    if (semctl(semid, BIN_WRITING, SETVAL, 0) == -1)
    {
        perror("Can't semctl (BIN_WRITING)");
        exit(1);
    }

    // читатели
    for (int i = 0; i < CNT_WRITER; ++i)
    {
        childpid[i] = fork();

        if (childpid[i] == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            writer(shared_num, semid);
        }
    }

    // писатели
    for (int i = CNT_WRITER; i < CNT_READER + CNT_WRITER; ++i)
    {
        childpid[i] = fork();

        if (childpid[i] == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            reader(shared_num, semid);
        }
    }

    for (int i = 0; i < CNT_READER + CNT_WRITER; i++)
    {
        pidw = waitpid(childpid[i], &wstatus, WUNTRACED);
        
        if (pidw == -1)
        {
            perror("Can't waitpid");
            exit(1);
        }

        if (WIFEXITED(wstatus))
            printf("Exited %d, status=%d\n", pidw, WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Killed %d by signal %d\n", pidw, WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Stopped %d by signal %d\n", pidw, WSTOPSIG(wstatus));
    }
    
    if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    {
        perror("Can't semctl.");
        exit(1);
    }
    if (shmdt(shared_num) == -1)
    {
        perror("Can't shmdt.");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("Can't shmctl.");
        exit(1);
    }
    return 0;
}