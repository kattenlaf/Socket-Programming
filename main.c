#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 2048
#define MAXIMUM_BACKLOG_CONNECTIONS 3 // https://man7.org/linux/man-pages/man2/listen.2.html

int main(void) {
    int server_fd, conn_socket; // File descriptor and new socket
    ssize_t conn_socket_data_read;
    struct sockaddr_in address;
    int opt;
    socklen_t addrlen = sizeof(address);
    char buffer[MAX_BUFFER_SIZE] = {0};
    char* data_from_server = "Server Message";

    // listen for a request
    // accept the connection
    // serve a response

    /* Set up socket and bind to port */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAXIMUM_BACKLOG_CONNECTIONS) < 0) {
            perror("listening failure");
            exit(EXIT_FAILURE);
        }

    printf("Webserver is listening...\n");
    if ((conn_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("Accept Failed");
        exit(EXIT_FAILURE);
    }

    // -1 for trailing null character to terminate string \0
    // https://man7.org/linux/man-pages/man2/read.2.html
    if ((conn_socket_data_read = read(conn_socket, buffer, MAX_BUFFER_SIZE - 1)) < 0) {
        perror("Failure reading data from client socket");
        exit(EXIT_FAILURE);
    }
    printf("Client sent %s\n", buffer);
    int socket_send = send(conn_socket, data_from_server, strlen(data_from_server), 0);
    if (socket_send < 0) {
        // Error sending data to client socket
        perror("Error sending data to client, socket send error");
        exit(EXIT_FAILURE);
    }
    printf("Sent message to client\n");

    // close client socket
    close(conn_socket);

    // close server socket
    close(server_fd);

    /*
    while (true) {

    }
    */

    return 0;
}