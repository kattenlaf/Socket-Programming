#include "server.h"
#ifndef __CLIENT_H
#define __CLIENT_H

typedef struct Connect_Send {
    int client_fd;
    char* message;
    struct sockaddr_in serveraddress;
} Connect_Send;

// definitions
// Must use void* for thread execution arguments
// multi threaded send requests
void* connect_send_message(void* args) {
    printf("Thread id is -> %d\n", pthread_self());
    Connect_Send* cs = (Connect_Send*)args;
    int status = connect(cs->client_fd, (struct sockaddr*)&cs->serveraddress, sizeof(cs->serveraddress));
    if (status < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    int sent = send(cs->client_fd, cs->message, strlen(cs->message), 0);
    if (sent < 0) {
        perror("Error sending message to server");
        exit(EXIT_FAILURE);
    } else {
        printf("Number of bytes sent: %i\n", sent);
    }
    exit(EXIT_SUCCESS);
}

#endif