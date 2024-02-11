#include "apue.h"

typedef int Myfunc(const char *, const struct stat *, int);

static Myfunc myfunc;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *);
static void print_tree(const char* dirname, int indent);

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

int main(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
        err_quit("Использование: ftw <начальный_каталог>");
    
    ret = myftw(argv[1], myfunc); /* выполняет всю работу */
    
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1;/* во избежание деления на 0; вывести 0 для всех счетчиков */
    printf("обычные файлы = %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
    printf("каталоги = %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
    printf("специальные файлы блочных устройств = %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
    printf("специальные файлы символьных устройств = %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
    printf("FIFO = %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
    printf("символические ссылки = %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
    printf("сокеты = %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
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
static int myftw(char *pathname, Myfunc *func)
{
    int len;
    fullpath = path_alloc(&len); /* выделить память для PATH_MAX+1 байт */
    /* (листинг 2.3) */
    strncpy(fullpath, pathname, len); /* защита от */
    fullpath[len - 1] = 0;
    /* переполнения буфера */
    return(dopath(func));
}
/*
* Обход дерева каталогов, начиная с "fullpath". Если "fullpath" не является
* каталогом, для него вызывается lstat(), func() и затем выполняется возврат.
* Для каталогов производится рекурсивный вызов функции.
*/
/* возвращаем то, что вернула функция func() */
static int dopath(Myfunc* func)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret;
    char *ptr;

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
    
    ptr = fullpath + strlen(fullpath); /* установить указатель */
    /* в конец fullpath */
    *ptr++ = '/';
    *ptr = 0;

    if ((dp = opendir(fullpath)) == NULL) /* каталог недоступен */
        return(func(fullpath, &statbuf, FTW_DNR));
    
    while ((dirp = readdir(dp)) != NULL) 
    {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue;
        /* пропустить каталоги "." и ".." */
        strcpy(ptr, dirp->d_name);
        /* добавить имя после слэша */
        if ((ret = dopath(func)) != 0) /* рекурсия */
            break;
        /* выход по ошибке */
    }

    ptr[-1] = 0; /* стереть часть строки от слэша и до конца */
    if (closedir(dp) < 0)
    err_ret("невозможно закрыть каталог %s", fullpath);
    return(ret);
}

static int myfunc(const char *pathname, const struct stat *statptr, int type)
{
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
            case S_IFREG: nreg++; break;
            case S_IFBLK: nblk++; break;
            case S_IFCHR: nchr++; break;
            case S_IFIFO: nfifo++; break;
            case S_IFLNK: nslink++; break;
            case S_IFSOCK: nsock++; break;
            case S_IFDIR:
                err_dump("признак S_IFDIR для %s", pathname);
                /* каталоги должны иметь тип = FTW_D */
        }
    break;

    case FTW_D:
        ndir++;
        break;

    case FTW_DNR:
        err_ret("закрыт доступ к каталогу %s", pathname);
        break;

    case FTW_NS:
        err_ret("ошибка вызова функции stat для %s", pathname);
        break;

    default:
        err_dump("неизвестный тип %d для файла %s", type, pathname);
    }

    return(0);
}