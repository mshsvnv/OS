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
#define SLEEP_TIME 15

sigset_t mask;

void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit r_limit;
    struct sigaction sa;

    umask(0); 

    if (getrlimit(RLIMIT_NOFILE, &r_limit) < 0) 
    { 
        syslog(LOG_ERR, "Невозможно получить максимальный номер дескриптора файла\n");
    }

    if ((pid = fork()) == -1)
    {
        syslog(LOG_ERR, "Ошибка вызова fork\n");
    } 
    else if (pid != 0)
    {
        exit(0);
    }

    if (setsid() == -1) 
    {
        syslog(LOG_ERR, "Ошибка вызова setsid\n");
        exit(1);
    }

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        syslog(LOG_ERR, "Невозможно игнорировать сигнал SIGHUP\n");
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

    fd0 = open("/dev/null", O_RDWR); 
    fd1 = dup(0);
    fd2 = dup(0);
   
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "Ошибочные файловые дескрипторы %d, %d, %d\n", fd0, fd1, fd2);
        exit(1);
    }
}

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
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

    if ((fd = fopen("/etc/syslog.conf", "r")) == NULL)
    {
        syslog(LOG_ERR, "Ошибка открытия файла /etc/syslog.conf.");
    }

    fscanf(fd, "%d", &pid);
    fclose(fd);
    syslog(LOG_INFO, "Pid: %d", pid);
}

void *thr_fn(void *arg)
{
    int err, signo;
    if (arg != NULL)
    {
        for (int i = 0; i < 5; i++)
        {
            syslog(LOG_INFO, "%s", (char*)arg);
            sleep(1);
        }
    }
    
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
        syslog(LOG_INFO, "Получен сигнал %d\n", signo);
        break;
    }

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int err, s1, s2;
	pthread_t tid;
    struct sigaction sa;
    time_t timestamp;
    struct tm *time_info;
    char *cmd;

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

    syslog(LOG_DAEMON, "Логин после daemonize: %s", getlogin());
    
    pthread_t tid_1, tid_2;
    err = pthread_create(&tid_1, NULL, thr_fn, "aaa");
    if (err != 0)
    {
        fprintf(stderr, "Невозможно создать поток 1");
        exit(1);
    }

    err = pthread_create(&tid_2, NULL, thr_fn, "bbb");
    if (err != 0)
    {
        fprintf(stderr, "Невозможно создать поток 2");
        exit(1);
    }

    void *res;
    s1 = pthread_join(tid_1, &res);
    // s2 = pthread_join(tid_2, &res);

    for (;;)
    {
        time(&timestamp);
        time_info = localtime(&timestamp);
        syslog(LOG_DAEMON, "%s", asctime(time_info)); 
        sleep(SLEEP_TIME);
    }

    return 0;
}