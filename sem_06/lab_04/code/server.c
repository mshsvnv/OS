#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "mymsg.h"

/* #define SOCK_NAME "server.soc" */
#define MAX_EVENTS 10

int listen_sock;

void sig_handler(int signal)
{
    printf("\nCaught signal = %d\n", signal);
    close(listen_sock);
    exit(0);
}

void setnonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        exit(1);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(1);
    }
}

int main()
{
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;
    struct sigaction sa;
    struct sockaddr_in srvr_addr;
    unsigned int addrlen;

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_sock < 0)
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

    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_addr.s_addr = INADDR_ANY;
    srvr_addr.sin_port = htons(5000);

    if (bind(listen_sock, (struct sockaddr *)&srvr_addr, sizeof(srvr_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(listen_sock, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(1);
    }

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(1);
    }

    // In which events the caller is interested in
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(1);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(1);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == listen_sock)
            {
                conn_sock = accept(listen_sock, (struct sockaddr *)&srvr_addr, &addrlen);

                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(1);
                }
                
                setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    exit(1);
                }
                else
                {
                    int done = 0;
                    while (1)
                    {
                        ssize_t count;
                        char buf[1024];
                       
                        count = read(conn_sock, buf, sizeof(buf));
                        
                        if (count > 0)
                        {
                            printf("%s (%d): %s\n",
                                    ((struct mymsg *)buf)->name,
                                    ((struct mymsg *)buf)->pid,
                                    ((struct mymsg *)buf)->data);
                        }
                        else if (count == -1)
                        {
                            if (errno != EAGAIN)
                            {
                                perror("read");
                                done = 1;
                            }
                            break;
                        }
                        else if (count == 0)
                        {
                            done = 1;
                            break;
                        }
                        
                        if (write(conn_sock,
                                    ((struct mymsg *)buf)->name,
                                    32) == -1)
                        {
                            perror("write");
                            exit(1);
                        }
                    }

                    if (done)
                    {
                        printf("close fd %d conn\n", conn_sock);
                        close(conn_sock);
                    }
                }
            }
        }
    }

    return 0;
}