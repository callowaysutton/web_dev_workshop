#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_EVENTS 10
#define READ_BUFFER 1024
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s"

// Set a socket to be non-blocking
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return -1;
    return 0;
}

// Handle incoming HTTP requests
void handle_client_request(int client_fd) {
    char buffer[READ_BUFFER];
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        // Respond with the request echoed back
        char response[READ_BUFFER * 2];
        int response_length = snprintf(response, sizeof(response), HTTP_RESPONSE, bytes_read, buffer);
        write(client_fd, response, response_length);
    }
}

int main() {
    int listen_fd, epoll_fd;
    struct epoll_event ev, events[MAX_EVENTS];

    // Create a non blocking socket for listening
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) exit(EXIT_FAILURE);
    if (set_nonblocking(listen_fd) == -1) exit(EXIT_FAILURE);

    // Bind the socket to an address and port
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(8080)
    };

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) exit(EXIT_FAILURE);

    // Start listening for incoming connections
    if (listen(listen_fd, SOMAXCONN) == -1) exit(EXIT_FAILURE);
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) exit(EXIT_FAILURE);

    // Add the listening socket to the epoll instance
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) exit(EXIT_FAILURE);

    // Main event loop
    printf("Echo server listening on port 8080...\n");
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) break;

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listen_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd == -1) continue;
                if (set_nonblocking(client_fd) == -1) continue;

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                    close(client_fd);
                    continue;
                }
            } else {
                int client_fd = events[i].data.fd;
                handle_client_request(client_fd);
                close(client_fd);
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}