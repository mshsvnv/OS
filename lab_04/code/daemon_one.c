#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <time.h>
#include <pthread.h>

#define CURDIR "/home/human/University/os/lab_demon/"
#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;            // блокировка на запист
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return fcntl(fd, F_SETLK, &fl);
}

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        fprintf(stderr, "%s: невозможно получить максимальный номер дескриптора", cmd);
        exit(1);
    }

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "%s: ошибка вызова функции fork", cmd);
        exit(1);
    }
    else if (pid != 0)
    {
        exit(0);
    }

    setsid();
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        fprintf(stderr, "Невозможно игнорировать сигнал SIGHUP");
        exit(1);
    }

    if (chdir("/") < 0)
    {
        fprintf(stderr, "Невозможно сделать текущим рабочим каталогом /");
        exit(1);
    }

    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "Ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}

int already_running()
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
    if (fd < 0)
    {
        syslog(LOG_ERR, "Невозможно открыть %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "Невозможно установить блокировку на %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return 0;
}

sigset_t mask;

/* void reread() {} */

void *thr_fn(void *arg)
{
    int err, signo;
    for (;;)
    {
        err = sigwait(&mask, &signo);
        if (err != 0)
        {
            syslog(LOG_ERR, "Ошибка вызова функции sigwait");
            exit(1);
        }
        switch (signo)
        {
            case SIGHUP:
                syslog(LOG_INFO, "Чтение конфигурационного файла");
                /* reread(); */
                break;
            case SIGTERM:
                syslog(LOG_INFO, "Получен сигнал SIGTERM; выход");
                exit(0);
            default:
                syslog(LOG_INFO, "Получен непредвиденный сигнал %d\n", signo);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;

    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;

    printf("Логин до daemonize: %s\n", getlogin());

    daemonize(cmd);

    if (already_running())
    {
        syslog(LOG_ERR, "Демон уже запущен");
        exit(1);
    }

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        fprintf(stderr, "Невозможно восстаносить действие SIG_DFL для SIGHUP");
        exit(1);
    }
    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        fprintf(stderr, "Ошибка выполнения операции SIG_BLOCK");
        exit(1);
    }
    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0)
    {
        fprintf(stderr, "Невозможно создать поток");
        exit(1);
    }

    syslog(LOG_INFO, "Логин после daemonize: %s\n", getlogin());

    for (;;)
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        syslog(LOG_INFO, "%s: Текущее время: %02d:%02d:%02d\n", cmd, tm.tm_hour, tm.tm_min, tm.tm_sec);
        sleep(1);
    }

    return 0;
}