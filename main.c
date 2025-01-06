#include "server.h" // https://man7.org/linux/man-pages/man2/listen.2.html

int main(void) {
    int server_fd, conn_socket; // File descriptor and new socket
    ssize_t conn_socket_data_read;
    struct sockaddr_in address;
    int opt;
    socklen_t addrlen = sizeof(address);
    char buffer[MAX_BUFFER_SIZE] = {0};
    char* data_from_server = "Server Message";

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

    short num_requests_sent = 0;

    // When a request comes in, spawn thread to handle the request, respond, 
    // update requests received
    // TODO implement multi threadedness to serve multiple requests at once
    while (true && num_requests_sent < 3) {
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
        num_requests_sent++;
    }

    // close client socket
    close(conn_socket);

    // close server socket
    close(server_fd);

    exit(EXIT_SUCCESS);
}