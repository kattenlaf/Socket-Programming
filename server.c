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

    while (true) {
        if (listen(server_fd, MAXIMUM_BACKLOG_CONNECTIONS) < 0) {
            perror("listening failure");
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "Webserver is listening...\n");
        fflush(stdout);
        // https://man7.org/linux/man-pages/man2/accept.2.html
        if ((conn_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Accept Failed");
            exit(EXIT_FAILURE);
        }

        Connect_Send cs;
        cs.socketfd = conn_socket;
        fprintf(stdout, "Connected socket for client: %d\n", cs.socketfd);
        fflush(stdout);
        pthread_t handle_connection_th;
        pthread_create(&handle_connection_th, NULL, &connect_send_message_server, &cs);
    }

    // close newly created connected socket
    close(conn_socket);
    // close server listening socket
    close(server_fd);

    exit(EXIT_SUCCESS);
}