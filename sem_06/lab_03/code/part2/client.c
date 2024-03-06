#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCK_NAME "socket.soc"
#define BUF_SIZE 256

int main(int argc, char **argv)
{
    int sock_fd;
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    char buf[BUF_SIZE];
    struct sockaddr srvr_addr, cln_addr;
    int namelen, bytes;

    if (sock_fd < 0) 
    {
        perror("socket failed");
        exit(1);
    }

    srvr_addr.sa_family = AF_UNIX;
    strcpy(srvr_addr.sa_data, SOCK_NAME);

    cln_addr.sa_family = AF_UNIX;
    sprintf(cln_addr.sa_data, "%d.soc", getpid());

    if (bind(sock_fd, &cln_addr, sizeof(cln_addr)) < 0) 
    {
        perror("bind failed");
        exit(1);
    }

    int pid = getpid();
    sprintf(buf, "PID %d", pid);

    if (sendto(sock_fd, buf, sizeof(buf), 0, &srvr_addr, sizeof(srvr_addr)) < 0)
    {
        perror("Can't sendto");
        exit(1);
    }

    char receive[BUF_SIZE];
    bytes = recvfrom(sock_fd, receive, sizeof(receive), 0, &srvr_addr, &namelen);

    if (bytes < 0) 
    {
        perror("recvfrom failed");
        exit(1);
    }

    printf("server to client: %s\n", receive);

    unlink(cln_addr.sa_data);
    close(sock_fd);
    return 0;
}