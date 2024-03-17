#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <memory.h>
#include "mymsg.h"
#include <signal.h>

int sockfd;

void sig_handler(int signal)
{
    printf("\nCaught signal = %d\n", signal);
    close(sockfd);
    exit(0);
}

int main()
{
    struct sigaction sa;
    struct sockaddr_in serv_addr;
    struct mymsg msg;
    char recv_data[1024];
    int recv_size;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0)
    {
        perror("sigaction");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5000);

    memset(&msg, 0, 1024);
    msg.pid = getpid();

    printf("PID: %d\n", msg.pid);
    printf("Input username: ");
    fgets(msg.name, 32, stdin);
    *strrchr(msg.name, '\n') = 0;

    printf("Input message: ");
    fgets(msg.data, 987, stdin);
    *strrchr(msg.data, '\n') = 0;

    memset(recv_data, 0, 1024);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    write(sockfd, &msg, 1024);

    if ((recv_size = read(sockfd, recv_data, 1024)) < 0)
        {
            perror("read");
            exit(1);
        }

        recv_data[1023] = 0;
        printf("Server: %s\n", recv_data);

    close(sockfd);

    return 0;
}