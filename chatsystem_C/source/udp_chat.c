#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_BUF 1024
#define MAX_EVENTS 16
#define MY_PORT 8080 
#define TARGET_PORT 8080

#define COL_RESET "\033[0m"
#define COL_GREEN "\033[32m" 
#define COL_CYAN  "\033[36m" 
#define CLEAR_LINE "\033[2K\r" 

void send_msg(int sock, char *msg, char *dst_ip) {
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(TARGET_PORT);
    target_addr.sin_addr.s_addr = inet_addr(dst_ip);

    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
        perror("sendto");
    }
}

void recv_and_print(int sock) {
    char buffer[MAX_BUF];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);
    if (len <= 0) return;

    buffer[len] = '\0';
    printf(CLEAR_LINE);
    printf("%s[%s]: %s%s\n", COL_GREEN, inet_ntoa(sender_addr.sin_addr), buffer, COL_RESET);
    printf("Input > ");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Dest IP>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MY_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        return 1;
    }

    int epfd = epoll_create(MAX_EVENTS);
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);

    printf("%s=== UDP P2P Chat Started (Port %d) ===%s\n", COL_CYAN, MY_PORT, COL_RESET);
    printf("Input > ");
    fflush(stdout);

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                char msg[MAX_BUF];
                if (fgets(msg, sizeof(msg), stdin) != NULL) {
                    msg[strcspn(msg, "\n")] = 0;
                    if (strlen(msg) > 0) {
                        send_msg(sock, msg, argv[1]);
                    }
                    printf("Input > ");
                    fflush(stdout);
                }
            } else if (events[i].data.fd == sock) {
                recv_and_print(sock);
            }
        }
    }

    close(sock);
    return 0;
}
