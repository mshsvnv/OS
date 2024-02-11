#include "apue.h"
#include <time.h>

typedef int Myfunc(const char *, const struct stat *, int);
typedef int DoPath(Myfunc *);

static Myfunc myfunc;
static int myftw(char *, Myfunc *, DoPath *);

static int dopath_no(Myfunc *);
static int dopath_chdir(Myfunc *);
void compare(char *);

static char print = 1;
static int level = 0;

int main(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
        err_quit("Использование: ftw <начальный_каталог>");
    
    // ret = myftw(argv[1], myfunc, dopath_no); /* выполняет всю работу */
    
    compare(argv[1]);

    exit(ret);
}
/*
* Обойти дерево каталогов, начиная с каталога "pathname".
* Пользовательская функция func() вызывается для каждого встреченного файла.
*/
#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4
/* файл, не являющийся каталогом */
/* каталог */
/* каталог, который не доступен для чтения */
/* файл, информацию о котором */
/* невозможно получить с помощью stat */
static char *fullpath; /* полный путь к каждому из файлов */
/* возвращаем то, что вернула функция func() */
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
    return(dopath(func)); 
}
/*
* Обход дерева каталогов, начиная с "fullpath". Если "fullpath" не является
* каталогом, для него вызывается lstat(), func() и затем выполняется возврат.
* Для каталогов производится рекурсивный вызов функции.
*/
/* возвращаем то, что вернула функция func() */

static int dopath_no(Myfunc* func)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;

    if (lstat(fullpath, &statbuf) < 0) /* ошибка вызова функции stat */
        return(func(fullpath, &statbuf, FTW_NS));

    if (S_ISDIR(statbuf.st_mode) == 0) /* не каталог */
        return(func(fullpath, &statbuf, FTW_F));
    /*
    * Это каталог. Сначала вызовем функцию func(),
    * а затем обработаем все файлы в этом каталоге.
    */
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);
    
    n = strlen(fullpath); /* установить указатель */

    if (n + NAME_MAX + 2 > path_len)
    {
        /* увеличить размер буфера */ 
        path_len *= 2;
        if ((fullpath = realloc(fullpath, path_len)) == NULL)
            err_sys("ошибка вызова realloc"); 
    } 

    fullpath[n++] = '/';
    fullpath[n] = 0;

    if ((dp = opendir(fullpath)) == NULL) /* каталог недоступен */
        return(func(fullpath, &statbuf, FTW_DNR));
    
    int stop = 1;
    level++;
    while ((dirp = readdir(dp)) != NULL && stop) 
    {
        if (!(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
        {
            strcpy(&fullpath[n], dirp->d_name);
           
            if ((ret = dopath_no(func)) != 0)
                stop = 0;
        }
    }
    level--;
    
    fullpath[n - 1] = 0;

    if (closedir(dp) < 0)
        err_ret("невозможно закрыть каталог %s", fullpath);
    return(ret);
}

static int dopath_chdir(Myfunc* func)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;

    if (lstat(fullpath, &statbuf) < 0) /* ошибка вызова функции stat */
        return(func(fullpath, &statbuf, FTW_NS));

    if (S_ISDIR(statbuf.st_mode) == 0) /* не каталог */
        return(func(fullpath, &statbuf, FTW_F));
    /*
    * Это каталог. Сначала вызовем функцию func(),
    * а затем обработаем все файлы в этом каталоге.
    */
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);

    if (chdir(fullpath) < 0) 
        err_sys("не удалось изменить директорию"); 

    fullpath[0] = '.';
    fullpath[1] = '/';
    fullpath[2] = 0;
    
    if ((dp = opendir(fullpath)) == NULL) /* каталог недоступен */
        return(func(fullpath, &statbuf, FTW_DNR));
    
    int stop = 1;
    level++;
    while ((dirp = readdir(dp)) != NULL && stop) 
    {
        if (!(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
        {
            strcpy(&fullpath[2], dirp->d_name);
           
            if ((ret = dopath_chdir(func)) != 0)
                stop = 0;
        }
    }
    level--;
    
    chdir("..");

    if (closedir(dp) < 0)
        err_ret("невозможно закрыть каталог %s", fullpath);
    
    return(ret);
}

static int myfunc(const char *pathname, const struct stat *statptr, int type)
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
                filename = strrchr(pathname, '/') + 1;
                printf("%s\n", filename);
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

void compare(char *pathname) 
{       
    print = 0;

    clock_t start, end;
    double cpu_time_used;

    int reps = 100;

    start = clock();
    for (int i = 0; i < reps; ++i) {}
        myftw(pathname, myfunc, dopath_no);
    end = clock();

    cpu_time_used = ((double) (end - start)) / (reps * CLOCKS_PER_SEC);
    printf("\nВремя обхода дерева каталогов без chdir: %f\n", cpu_time_used);

    start = clock();
    for (int i = 0; i < reps; ++i) {}
        myftw(pathname, myfunc, dopath_chdir);
    end = clock();

    cpu_time_used = ((double) (end - start)) / (reps * CLOCKS_PER_SEC);
    printf("Время обхода дерева каталогов с chdir: %f\n", cpu_time_used);
}