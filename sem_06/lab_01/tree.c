#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>

void print_tree(const char* dirname, int indent)
{
    DIR *pdir = opendir(dirname);
    struct dirent *pd;
    struct stat sbuf;
    chdir(dirname);

    while((pd = readdir(pdir)) != NULL)
    {
        lstat(pd->d_name, &sbuf);

        if(strcmp(pd->d_name, ".") == 0 || 
                strcmp(pd->d_name, "..") == 0)
        {
            continue;
        }
        int depth = indent;
        while(depth--)
            printf("| _");
        printf("%s\n", pd->d_name);   
        depth = indent;
        if(S_ISDIR(sbuf.st_mode))
        {
            print_tree(pd->d_name, depth+2);
            printf("|\n");
        }

    }
    chdir("..");
    closedir(pdir);
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("usage: ./tree dir\n");
        exit(1);
    }
    printf("%s\n", argv[1]);
    printf("|\n");
    print_tree(argv[1], 1);
    printf("\n");
    return 0;
}