#ifndef __MYMSG_H__
#define __MYMSG_H__

#define BUF_SIZE 256

struct mymsg {
    int pid;
    char data[BUF_SIZE - sizeof(int)];
};

#endif // __MYMSG_H__