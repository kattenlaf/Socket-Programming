#include "shared.h"
#ifndef __SERVER_H
#define __SERVER_H
#define MAXIMUM_BACKLOG_CONNECTIONS 3

char* data_from_server = "Server Message";
// declarations
void* connect_send_message_server(void* args);

// When a request comes in, spawn thread to handle the request, respond, 
    // update requests received
    // TODO implement multi threadedness to serve multiple requests at once

// definitions
void* connect_send_message_server(void* args) {
    Connect_Send* cs = (Connect_Send*)args;
    ssize_t data_read;
    char buffer[MAX_BUFFER_SIZE] = {0};
    fprintf(stdout, "socket value is %d\n", cs->socketfd);
    fflush(stdout);
    if ((data_read = read(cs->socketfd, buffer, MAX_BUFFER_SIZE - 1)) < 0) {
        perror("Failure reading data from client socket");
        close_fd_return(cs->socketfd);
    }
    fprintf(stdout, "bytes read on server side:%d\nClient sent \n%s\n", data_read, buffer);
    fflush(stdout);
    _ssize_t bytes_sent = send(cs->socketfd, data_from_server, strlen(data_from_server), 0);
    if (bytes_sent < 0) {
        // Error sending data to client socket
        perror("Error sending data to client, socket send error");
        close_fd_return(cs->socketfd);
    }
    fprintf(stdout, "Bytes sent: %d\nSent message to client\n", bytes_sent);
    fflush(stdout);
    close_fd_return(cs->socketfd);
}

#endif