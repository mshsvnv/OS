#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCK_NAME "socket.soc"
#define BUF_SIZE 256

int main(int argc, char ** argv)
{
    int sock_fd;
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    char buf[BUF_SIZE];
    struct sockaddr srvr_addr;

    if (sock_fd < 0) 
    {
        perror("socket failed");
        exit(1);
    }

    srvr_addr.sa_family = AF_UNIX;
    strcpy(srvr_addr.sa_data, SOCK_NAME);

    int pid = getpid();
    sprintf(buf, "client PID: %d", pid);

    if (sendto(sock_fd, buf, strlen(buf), 0, &srvr_addr, sizeof(srvr_addr)) < 0)
    {
        perror("Can't sendto");
        exit(1);
    }

    close(sock_fd);
    
    return 0;
}