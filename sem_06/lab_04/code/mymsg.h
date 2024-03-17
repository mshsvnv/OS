#ifndef __MYMSG_H__
#define __MYMSG_H__

struct mymsg {
    int pid;
    char name[32];
    char data[1024 - sizeof(int) - sizeof(char[32])];
};

#endif // __MYMSG_H__