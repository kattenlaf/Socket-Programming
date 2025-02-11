#include "shared.h"

#ifndef __CLIENT_H
#define __CLIENT_H

// definitions
// Must use void* for thread execution arguments
// multi threaded send requests

void* connect_send_message(void* args) {
    pthread_t tid = pthread_self();
    fprintf(stdout, "Thread id is -> %d\n", tid);
    fflush(stdout);
    char message_to_send[MAX_BUFFER_SIZE] = {0};
    sprintf(message_to_send, "Hello from client with threadid %d\n", tid);

    Connect_Send* cs = (Connect_Send*)args;
    int connection_socket = cs->socketfd;
    cs->message = message_to_send;
    int status = connect(connection_socket, (struct sockaddr*)&cs->serveraddress, sizeof(cs->serveraddress));
    if (status < 0) {
        perror("Error connecting to server\n");
        close_fd_return(connection_socket);
    }
    fprintf(stdout, "Connection established!\n");
    fflush(stdout);
    // Send message to server
    _ssize_t sent = send(connection_socket, cs->message, strlen(cs->message), 0);
    if (sent < 0) {
        perror("Error sending message to server\n");
        close_fd_return(connection_socket);
    }
    // Read response from server
    char* buffer = (char*)malloc(MAX_BUFFER_SIZE*sizeof(char));
    _ssize_t msg_read_status = read(connection_socket, buffer, MAX_BUFFER_SIZE - 1);
    if (msg_read_status < 0) {
        perror("Failure reading message responded from server\n");
        free(buffer);
        close_fd_return(connection_socket);
    }
    fprintf(stdout, "Message received from server and number of bytes:\n%s - {%d} \n", buffer, msg_read_status);
    fflush(stdout);
    free(buffer);
    close_fd_return(connection_socket);
}
#endif