#include "apue.h"
#include <time.h>

typedef int Myfunc(const char *, int, int);
typedef int DoPath(Myfunc *, int *);

static Myfunc myfunc;
static int myftw(char *, Myfunc *, DoPath *);

static int dopath_no(Myfunc *, int *);
static int dopath_chdir(Myfunc *, int *);
void compare(char *);

char print = 1;
int amount_realloc = 0;

int main(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
        err_quit("Использование: ftw <начальный_каталог>");
    
    ret = myftw(argv[1], myfunc, dopath_chdir); 

    print = 0;
    
    compare(argv[1]);

    exit(ret);
}

#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4

static char *fullpath; 
static size_t path_len;

static int myftw(char *pathname, Myfunc *func, DoPath *dopath)
{
    if (path_len <= strlen(pathname))
    { 
        path_len = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, path_len)) == NULL) 
            err_sys("ошибка вызова realloc"); 
    } 

    strcpy(fullpath, pathname);

    int level = 0;
    return(dopath(func, &level)); 
}

static int dopath_no(Myfunc* func, int *level)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;

    if (lstat(fullpath, &statbuf) < 0) 
        return(func(fullpath, *level, FTW_NS));

    if (S_ISDIR(statbuf.st_mode) == 0) 
        return(func(fullpath, *level, FTW_F));
   
    if ((ret = func(fullpath, *level, FTW_D)) != 0)
        return(ret);
    
    n = strlen(fullpath); 

    if (n + NAME_MAX + 2 > path_len)
    {
        path_len *= 2;

        if ((fullpath = realloc(fullpath, path_len)) == NULL)
            err_sys("ошибка вызова realloc"); 
        
        amount_realloc++;
    } 

    fullpath[n++] = '/';
    fullpath[n] = 0;

    if ((dp = opendir(fullpath)) == NULL) 
        return(func(fullpath, *level, FTW_DNR));
    
    int stop = 1;
    (*level)++;
    while ((dirp = readdir(dp)) != NULL && stop) 
    {
        if (!(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
        {
            strcpy(&fullpath[n], dirp->d_name);
           
            if ((ret = dopath_no(func, level)) != 0)
                stop = 0;
        }
    }
    (*level)--;
    
    fullpath[n - 1] = 0;

    if (closedir(dp) < 0)
        err_ret("невозможно закрыть каталог %s", fullpath);

    return(ret);
}

static int dopath_chdir(Myfunc* func, int *level)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;

    if (lstat(fullpath, &statbuf) < 0) 
        return(func(fullpath, *level, FTW_NS));

    if (S_ISDIR(statbuf.st_mode) == 0) 
        return(func(fullpath, *level, FTW_F));

    if ((ret = func(fullpath, *level, FTW_D)) != 0)
        return(ret);
    
    if ((dp = opendir(fullpath)) == NULL) 
        return(func(fullpath, *level, FTW_DNR));

    if (chdir(fullpath) < 0) 
        err_sys("не удалось изменить директорию"); 
    
    int stop = 1;
    (*level)++;
    while ((dirp = readdir(dp)) != NULL && stop) 
    {
        if (!(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
        {
            strcpy(&fullpath[0], dirp->d_name);
           
            if ((ret = dopath_chdir(func, level)) != 0)
                stop = 0;
        }
    }
    (*level)--;
    
    chdir("..");

    if (closedir(dp) < 0)
        err_ret("невозможно закрыть каталог %s", fullpath);
    
    return(ret);
}

static int myfunc(const char *pathname, int level, int type)
{
    if (print) 
    {    
        if (type == FTW_F || type == FTW_D) 
        {
            const char *filename;
            int i = 0;

            for (int i = 0; i < level; ++i)
            {
                if (i != level - 1)
                    printf("│   ");
                else
                    printf("└───");
            }
            
            if (level > 0) 
            {
                filename = strrchr(pathname, '/');

                if (filename != NULL)
                    printf("%s\n", filename + 1);
                else
                    printf("%s\n", pathname);
            } else 
                printf("%s\n", pathname);

        } else if (type == FTW_DNR) 
            err_ret("закрыт доступ к каталогу %s", pathname);
        else if (type == FTW_NS)
            err_ret("ошибка вызова функции stat для %s", pathname);
        else
            err_ret("неизвестный тип %d для файла %s", type, pathname);
    }

    return(0);
}

static long long getThreadCpuTimeNs()
{
    struct timespec t;

    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t))
    {
        perror("clock_gettime");
        return 0;
    }

    return t.tv_sec * 1000000000LL + t.tv_nsec;
}

void compare(char *pathname) 
{       
    print = 0;

    long long start, end;
    double cpu_time_used;

    int reps = 100;

    start = getThreadCpuTimeNs();
    for (int i = 0; i < reps; ++i) {}
        myftw(pathname, myfunc, dopath_no);
    end = getThreadCpuTimeNs();

    cpu_time_used = ((double) (end - start)) / reps;
    printf("\nВремя обхода дерева каталогов без chdir: %f\n", cpu_time_used);
    // printf("Колиество срабатываний realloc: %d\n\n", amount_realloc);

    start = getThreadCpuTimeNs();
    for (int i = 0; i < reps; ++i) {}
        myftw(pathname, myfunc, dopath_chdir);
    end = getThreadCpuTimeNs();

    cpu_time_used = ((double) (end - start)) / reps;
    printf("Время обхода дерева каталогов с chdir: %f\n", cpu_time_used);
}