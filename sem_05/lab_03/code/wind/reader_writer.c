#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <process.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#define READERS_NUM 5
#define WRITERS_NUM 3

HANDLE can_read;
HANDLE can_write;
HANDLE mutex;

HANDLE readers_threads[READERS_NUM];
HANDLE writers_threads[WRITERS_NUM];

int readers_id[READERS_NUM];
int writers_id[WRITERS_NUM];

long waiting_readers = 0;
long active_readers = 0;

long waiting_writers = 0;
long active_writer = false;

int value = 0;
int flag = 1;

void sig_handler(int sig_num)
{
    flag = 0;
    printf("\npid %d: catch signal %d\n", _getpid(), sig_num);
}

void start_read()
{
    InterlockedIncrement(&waiting_readers);

    if (active_writer || waiting_writers > 0)
    {
        WaitForSingleObject(can_read, INFINITE);
    }
    
    InterlockedDecrement(&waiting_readers);
    InterlockedIncrement(&active_readers);
    SetEvent(can_read);
    ReleaseMutex(mutex);
}

void stop_read()
{
    InterlockedDecrement(&active_readers);

    if (active_readers == 0)
    {
        SetEvent(can_write);
    }
}

void start_write()
{
    InterlockedIncrement(&waiting_writers);
    if (active_readers > 0 || active_writer)
    {
        WaitForSingleObject(can_write, INFINITE);
    }
  
    InterlockedIncrement(&active_writer);
    InterlockedDecrement(&waiting_writers);
}

void stop_write()
{
    ResetEvent(can_write);
    InterlockedDecrement(&active_writer);
    if (waiting_readers > 0)
    {
        SetEvent(can_read);
    }
    else
    {
        SetEvent(can_write);
    }
}


DWORD WINAPI reader(const LPVOID parametr)
{
    int id = *(int*)parametr;

    srand(GetCurrentThreadId());
    while(flag)
    {
        int sleep_time = (rand() % 1000);
        Sleep(sleep_time);

        start_read();
        
        printf("Reader(%d) %2d read  %2d\n", GetCurrentThreadId(), id, value);
        
        stop_read();
    }

    return 0;
}


DWORD WINAPI writer(const LPVOID parametr)
{
    int id = *(int*)parametr;

    srand(GetCurrentThreadId());
    while(flag)
    {
        int sleep_time = (rand() % 1000);
        Sleep(sleep_time);

        start_write();
    
        value++;
        printf("Writer(%d) %2d write %2d\n", GetCurrentThreadId(), id, value);
 
        stop_write();
    }

    return 0;
}

int main(void)
{
    printf("pid %d\n", _getpid());
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }

    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL)
    {
        perror("CreateMutex");
        exit(1);
    }

    can_write = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (can_write == NULL)
    {
        perror("CreateEvent");
        exit(1);
    }

    can_read = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (can_read == NULL)
    {
        perror("CreateEvent");
        exit(1);
    }

    for (int i = 0; i < READERS_NUM; i++)
    {
        readers_id[i] = i;
        readers_threads[i] = CreateThread(NULL, 0, reader, readers_id + i, 0, NULL);

        if (readers_threads[i] == NULL)
        {
            perror("CreateEvent");
            exit(1);
        }
    }

    for (int i = 0; i < WRITERS_NUM; i++)
    {
        writers_id[i] = i;
        writers_threads[i] = CreateThread(NULL, 0, writer, writers_id + i, 0, NULL);

        if (writers_threads[i] == NULL)
        {
            perror("CreateThread");
            exit(1);
        }
    }

    WaitForMultipleObjects(READERS_NUM, readers_threads, TRUE, INFINITE);
    WaitForMultipleObjects(WRITERS_NUM, writers_threads, TRUE, INFINITE);

    for (size_t i = 0; i < READERS_NUM; i++)
    {
        DWORD status = WaitForSingleObject(readers_threads[i], INFINITE);
        printf("exit reader %d, status %d\n", i, status);
    }

    for (size_t i = 0; i < WRITERS_NUM; i++)
    {
        DWORD status = WaitForSingleObject(readers_threads[i], INFINITE);
        printf("exit writer %d, status %d\n", i, status);
    }

    for (int i = 0; i < READERS_NUM; i++)
    {
        CloseHandle(readers_threads[i]);
    }

    for (int i = 0; i < WRITERS_NUM; i++)
    {
        CloseHandle(writers_threads[i]);
    }

    CloseHandle(mutex);
    CloseHandle(can_write);
    CloseHandle(can_read);
}
