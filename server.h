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
    if ((data_read = read(cs->socketfd, buffer, MAX_BUFFER_SIZE - 1)) < 0) {
        perror("Failure reading data from client socket");
            exit(EXIT_FAILURE);
    }
    printf("bytes read on server side:%d\n", data_read);
    printf("Client sent \n%s\n", buffer);
    
    int bytes_sent = send(cs->socketfd, data_from_server, strlen(data_from_server), 0);
    if (bytes_sent < 0) {
        // Error sending data to client socket
        perror("Error sending data to client, socket send error");
        exit(EXIT_FAILURE);
    }
    printf("Bytes sent: %d\n", bytes_sent);
    printf("Sent message to client\n");
    close(cs->socketfd);
}

#endif