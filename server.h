#include "shared.h"
#include <sys/select.h>

#ifndef __SERVER_H
#define __SERVER_H
#define MAXIMUM_BACKLOG_CONNECTIONS 3
#define RESPONSE_MSG 4096

// declarations
ssize_t send_http_ok(int socket);

// definitions
ssize_t send_http_ok(int socket) {
    // build http ok message and send to client
    char response[RESPONSE_MSG];
    sprintf(response, 
            "HTTP/1.1 200 OK\n"
            "Date: Mon, 20 Jan 2025 00:00:00 GMT\n"
            "Server: Apache/2.2.3\n"
            "Last-Modified: Mon, 20 Jan 2025 00:00:00 GMT\n"
            "ETag: \"56d-9989200-1132c580\"\n"
            "Content-Type: text/html\n"
            "Content-Length: 15\n"
            "Accept-Ranges: bytes\n"
            "Connection: close\n"
            "\n"
            "Hello World!\0");
    ssize_t bytes_sent = send(socket, response, strlen(response), 0);
    return bytes_sent;
}
#endif