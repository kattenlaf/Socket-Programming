#include "shared.h"

#ifndef __CLIENT_H
#define __CLIENT_H

// definitions
// Must use void* for thread execution arguments
// multi threaded send requests
void* connect_send_message(void* args) {
    fprintf(stdout, "Thread id is -> %d\n", pthread_self());
    fflush(stdout);
    char message_to_send[MAX_BUFFER_SIZE] = {0};
    char buffer[MAX_BUFFER_SIZE] = {0};

    Connect_Send* cs = (Connect_Send*)args;
    cs->message = message_to_send;
    int status = connect(cs->socketfd, (struct sockaddr*)&cs->serveraddress, sizeof(cs->serveraddress));
    if (status < 0) {
        perror("Error connecting to server");
        pthread_exit(NULL);
    }
    // Send message to server
    int sent = send(cs->socketfd, cs->message, strlen(cs->message), 0);
    if (sent < 0) {
        perror("Error sending message to server");
        pthread_exit(NULL);
    } else {
        fprintf(stdout, "Number of bytes sent: %d\n", sent);
        fflush(stdout);
    }
    // Read response from server
    int msg_read_status = read(cs->socketfd, buffer, MAX_BUFFER_SIZE - 1);
    fprintf(stdout, "Bytes received %d\n", msg_read_status);
    if (msg_read_status < 0) {
        perror("Failure reading message responded from server");
        pthread_exit(NULL);
    }
    fprintf(stdout, "Message Received From Server Below:\n%s\n", buffer);
    fflush(stdout);
    close(cs->socketfd);
    pthread_exit(NULL);
}
#endif