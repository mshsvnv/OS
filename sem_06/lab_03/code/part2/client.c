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
    int sock;
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);

    char buf[BUF_SIZE];
    struct sockaddr srvr_name, rcvr_name;
    int namelen, bytes;

    if (sock < 0) 
    {
        perror("socket failed");
        exit(1);
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    rcvr_name.sa_family = AF_UNIX;
    sprintf(rcvr_name.sa_data, "%d.soc", getpid());

    if (bind(sock, &rcvr_name, sizeof(rcvr_name)) < 0) 
    {
        perror("bind failed");
        exit(1);
    }

    int pid = getpid();
    sprintf(buf, "PID %d", pid);

    if (sendto(sock, buf, sizeof(buf), 0, &srvr_name, sizeof(srvr_name)) < 0)
    {
        perror("Can't sendto");
        exit(1);
    }

    char receive[BUF_SIZE];
    bytes = recvfrom(sock, receive, sizeof(receive), 0, &srvr_name, &namelen);

    if (bytes < 0) 
    {
        perror("recvfrom failed");
        exit(1);
    }

    printf("server to client: %s\n", receive);

    close(sock);
    return 0;
}