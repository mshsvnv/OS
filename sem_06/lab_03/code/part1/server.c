#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define SOCK_NAME "socket.soc"
#define BUF_SIZE 256

int sock_fd;

void sig_handler()
{
    close(sock_fd);
    unlink(SOCK_NAME);
    exit(0);
}

int main(int argc, char **argv)
{
    struct sockaddr srvr_addr, cln_addr;
    char buf[BUF_SIZE];
    int namelen, bytes;

    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    
    if (sock_fd < 0) 
    {
        perror("socket failed");
        exit(1);
    }

    srvr_addr.sa_family = AF_UNIX;
    strcpy(srvr_addr.sa_data, SOCK_NAME);

    if (bind(sock_fd, &srvr_addr, sizeof(srvr_addr)) < 0) 
    {
        perror("bind failed");
        exit(1);
    }

    struct sigaction sa;

    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    
    for (;;)
    {
        bytes = recvfrom(sock_fd, buf, sizeof(buf), 0, &cln_addr, &namelen);
        
        if (bytes < 0) 
        {
            perror("recvfrom failed");
            exit(1);
        }
        
        printf("Read msg: %s\n", buf);
    }

    return 0;
}
 