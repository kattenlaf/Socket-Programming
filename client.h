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
    char buffer[MAX_BUFFER_SIZE] = {0};

    Connect_Send* cs = (Connect_Send*)args;
    int connection_socket = cs->socketfd;
    cs->message = message_to_send;
    int status = connect(connection_socket, (struct sockaddr*)&cs->serveraddress, sizeof(cs->serveraddress));
    if (status < 0) {
        perror("Error connecting to server");
        close_fd_return(connection_socket);
    }
    fprintf(stdout, "Connection established!\n");
    // Send message to server
    _ssize_t sent = send(connection_socket, cs->message, strlen(cs->message), 0);
    if (sent < 0) {
        perror("Error sending message to server");
        close_fd_return(connection_socket);
    } else {
        fprintf(stdout, "Number of bytes sent: %d\n", sent);
        fflush(stdout);
    }
    // Read response from server
    _ssize_t msg_read_status = read(connection_socket, buffer, MAX_BUFFER_SIZE - 1);
    fprintf(stdout, "Bytes received %d\n", msg_read_status);
    fflush(stdout);
    if (msg_read_status < 0) {
        perror("Failure reading message responded from server");
        close_fd_return(connection_socket);
    }
    fprintf(stdout, "Message Received From Server Below:\n%s\n", buffer);
    fflush(stdout);
    close_fd_return(connection_socket);
}
#endif