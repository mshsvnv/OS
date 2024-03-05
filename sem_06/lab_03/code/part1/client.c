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
    int sock;
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    char buf[BUF_SIZE];
    struct sockaddr srvr_name;

    if (sock < 0) 
    {
        perror("socket failed");
        exit(1);
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    int pid = getpid();
    sprintf(buf, "client PID: %d", pid);

    if (sendto(sock, buf, strlen(buf), 0, &srvr_name, strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) < 0)
    {
        perror("Can't sendto");
        exit(1);
    }

    close(sock);
    
    return 0;
}