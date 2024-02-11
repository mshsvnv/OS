#define _XOPEN_SOURCE 600 /* Single UNIX Specification, Version 3 */
#include <sys/types.h>
/* некоторые системы требуют этот заголовок */
#include <sys/stat.h>
#include <sys/termios.h>
/* структура winsize */
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif
#include <stdio.h>
/* для удобства */
#include <stdlib.h>
/* для удобства */
#include <stddef.h>
/* макрос offsetof */
#include <string.h>
/* для удобства */
#include <unistd.h>
/* для удобства */
#include <signal.h>

#include <dirent.h>
#include <limits.h>

#include <errno.h>
#include <stdarg.h>
/* определение переменной errno */
/* список аргументов переменной длины ISO C */

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif
#define SUSV3 200112L
static long posix_version = 0;

void err_sys(const char *fmt, ...);

/* Если константа PATH_MAX не определена, то нельзя гарантировать, */
/* что следующее число будет достаточно адекватным */
#define PATH_MAX_GUESS 1024
char * path_alloc(int *sizep) /* если результат не пустой указатель, */
/* то также возвращается размер выделенной памяти */
{
    char *ptr;
    int size;
    
    if (posix_version == 0)
        posix_version = sysconf(_SC_VERSION);

    if (pathmax == 0) { /* первый вызов функции */
        errno = 0;
        
        if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
            if (errno == 0)
                pathmax = PATH_MAX_GUESS; /* если константа не определена */
            else
                err_sys("ошибка вызова pathconf с параметром _PC_PATH_MAX");
            } else {
                pathmax++;
            /* добавить 1, т.к. путь относительно корня */
            }
    }

    if (posix_version < SUSV3)
        size = pathmax + 1;
    else
        size = pathmax;

    if ((ptr = malloc(size)) == NULL)
        err_sys("ошибка функции malloc");
    
    if (sizep != NULL)
        *sizep = size;
    return(ptr);
}

#define MAXLINE 4096

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[MAXLINE];
    vsnprintf(buf, MAXLINE, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), MAXLINE - strlen(buf), ": %s",
    strerror(error));
    strcat(buf, "\n");
    fflush(stdout); /* в случае, когда stdout и stderr – */
    /* одно и то же устройство */
    fputs(buf, stderr);
    fflush(NULL); /* сбрасывает все выходные потоки */
}

/*
* Обработка нефатальных ошибок, связанных с системными вызовами.
* Выводит сообщение и возвращает управление.
*/
void err_ret(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
}

void err_quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    err_doit(0, 0, fmt, ap);

    va_end(ap);
    exit(1);
}

/*
* Обработка фатальных ошибок, связанных с системными вызовами.
* Выводит сообщение, создает файл core и завершает работу процесса.
*/
void err_dump(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    abort();
    /* записать дамп памяти в файл и завершить процесс */
    exit(1);
    /* этот вызов никогда не должен быть выполнен */
}

/*
* Обработка фатальных ошибок, связанных с системными вызовами.
* Выводит сообщение и завершает работу процесса.
*/
void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);

    exit(1);
}