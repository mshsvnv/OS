#define _XOPEN_SOURCE 700

#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h> 
#include <stdio.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SLEEP_TIME 1

sigset_t mask;

int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return fcntl(fd, F_SETLK, &fl);
}

void daemonize(const char *name)
{
    pid_t pid;
    int fd0, fd1, fd2;
    struct rlimit r_limit;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &r_limit) < 0)
    {
        syslog(LOG_ERR, "Невозможно определить максимальный номер дескриптора файла\n");
    }

    if ((pid = fork()) == -1)
    {
        syslog(LOG_ERR, "Ошибка вызова fork\n");
    }
    else if (pid != 0)
    {
        exit(0);
    }
    
    sa.sa_handler = SIG_IGN; 
    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) 
    {
        syslog(LOG_ERR, "Невозможно игнорировать SIGHUP\n");
    }

    if (setsid() == -1)
    {
        syslog(LOG_ERR, "Ошибка вызова setsid\n");
        exit(1);
    }

    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "Невозможно изменить каталог на /\n");
    }
    if (r_limit.rlim_max == RLIM_INFINITY)
    {
        r_limit.rlim_max = 1024;
    }

    for (int i = 0; i < r_limit.rlim_max; i++)
    {
        close(i);
    }
    
    openlog(name, LOG_CONS, LOG_DAEMON);
    
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "Ошибка %d, %d, %d\n", fd0, fd1, fd2);
        exit(1);
    }
}

int already_running(void)
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0)
    {
        syslog(LOG_ERR, "Невозможно открыть %s : %s\n", LOCKFILE, strerror(errno));
        exit(1);
    }
    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1; 
        }
        syslog(LOG_ERR, "Ошибка lock %s: %s\n", LOCKFILE, strerror(errno));
        exit(1); 
    }
    
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);
    return 0;
}

void reread(void) 
{
    FILE *fd;
    int pid;
    syslog(LOG_DAEMON, "Логин: %s", getlogin());

    if ((fd = fopen("/etc/sysloп.conf", "r")) == NULL)
    {
        syslog(LOG_ERR, "Ошибка открытия файла /var/run/daemon.pid.");
    }

    fscanf(fd, "%d", &pid);
    fclose(fd);
    syslog(LOG_INFO, "PID = %d", pid);
}

void *thr_fn(void *arg)
{
    int err, signo;
    for (;;)
    {
        err = sigwait(&mask, &signo);
        
        if (err != 0)
        {
            syslog(LOG_ERR, "Ошибка вызова sigwait\n");
            exit(1);
        }
        
        switch (signo)
        {
            case SIGHUP:
                syslog(LOG_INFO, "Получен сигнал SIGHUP\n");
                reread();
                break;
            case SIGTERM:
                syslog(LOG_INFO, "Получен сигнал SIGTERM, выход\n");
                exit(0);
            default:
                syslog(LOG_INFO, "Получен непредвиденный сигнал %d\n", signo);
                break;
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    time_t timestamp;
    struct tm *time_info;
    char *cmd;
    int err;
    pthread_t tid;
    struct sigaction sa;

    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;

    daemonize(cmd);
    if (already_running())
    {
        syslog(LOG_ERR, "Демон уже запущен\n");
        exit(1);
    }

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        syslog(LOG_ERR, "Невозможно восстановить действие SIG_DFL для SIGHUP\n");
    }

    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        syslog(LOG_ERR, "Ошибка выполнения операции SIG_BLOCK\n");
    }

    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0)
    {
        syslog(LOG_ERR, "Невозможно создать поток\n");
    }
    else
    {
        syslog(LOG_INFO, "Создан новый поток\n");
    }
    syslog(LOG_DAEMON, "Логин: %s", getlogin());
    for (;;)
    {
        sleep(SLEEP_TIME);
        time(&timestamp);
        time_info = localtime(&timestamp);
        syslog(LOG_DAEMON, "%s", asctime(time_info));
    }

    return 0;
}
