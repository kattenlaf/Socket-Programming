#include "shared.h"
#include <sys/select.h>

#ifndef __SERVER_H
#define __SERVER_H
#define MAXIMUM_BACKLOG_CONNECTIONS 3
#define RESPONSE_MSG 4096

// declarations
ssize_t send_http_ok(int socket);
void set_fds(fd_set* readfds, int clientfds[], int* maxfd);

// definitions
ssize_t send_http_ok(int socket) {
    // build http ok message and send to client
    char response[RESPONSE_MSG];
    sprintf(response, 
            "HTTP/1.1 200 OK\n"
            "Date: Mon, 20 Jan 2025 00:00:00 GMT\n"
            "Server: Timbo/Server\n"
            "Last-Modified: Mon, 20 Jan 2025 00:00:00 GMT\n"
            "Content-Type: text/html\n"
            "Connection: close\n"
            "\n"
            "Hello World!\0");
    ssize_t bytes_sent = send(socket, response, strlen(response), 0);
    return bytes_sent;
}

// Summary
// Add file descriptors to the fd_set so that server will respond to the client
void set_fds(fd_set* readfds, int clientfds[], int* maxfd) {
    int sd;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sd = clientfds[i];
        FD_SET(sd, readfds);
        if (sd > *maxfd) {
            *maxfd = sd;
        }
    }
    if (sd > *maxfd) {
        *maxfd = sd;
    }
}

#endif