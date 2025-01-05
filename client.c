#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "server.h"

int main(void) {
    // Create a socket, send a request to the port server is listening on and accept a response?
    struct sockaddr_in server_address;
    char* message_to_server = "Hello server";
    char buffer[MAX_BUFFER_SIZE] = {0};

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Client Socket Creation Error");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    // https://man7.org/linux/man-pages/man3/inet_pton.3.html

    int conversion_status = inet_pton(AF_INET, LOCAL_HOST, &server_address.sin_addr);
    if (conversion_status < 0) {
        perror("Address is not valid");
        exit(EXIT_FAILURE);
    } else if (conversion_status == 0) {
        perror("Not a valid character string representing network address");
        exit(EXIT_FAILURE);
    }

    int status = connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (status < 0) {
        perror("Connection to server failure");
        exit(EXIT_FAILURE);
    }

    send(client_fd, message_to_server, strlen(message_to_server), 0);
    // Read response from server
    int msg_read_status = read(client_fd, buffer, MAX_BUFFER_SIZE - 1);
    if (msg_read_status < 0) {
        perror("Failure reading message responded from server");
        exit(EXIT_FAILURE);
    }

    printf("Message Received From Server Below:\n%s\n", buffer);

    close(client_fd);
    exit(EXIT_SUCCESS);
}