#include "server.h" // https://man7.org/linux/man-pages/man2/listen.2.html

int main(void) {
    int server_fd, connected_fd; // File descriptor and new socket created on acceptance
    struct sockaddr_in address;
    int opt;
    socklen_t addrlen = sizeof(address);
    char buffer[MAX_BUFFER_SIZE] = {0};

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

    // Server socket only needs to listen once
    if (listen(server_fd, MAX_THREADS) < 0) {
        perror("listening failure");
        exit(EXIT_FAILURE);
    }

    while (true) {
        // https://man7.org/linux/man-pages/man2/accept.2.html
        if ((connected_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Accept Failed");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Webserver is waiting to accept request...\n");
        fflush(stdout);

        Connect_Send* cs = (Connect_Send*)malloc(sizeof(Connect_Send));
        cs->socketfd = connected_fd;

        /* When done this way the same memory address is used for the variable
        and then a concurrent issue occurs. If I want to use stack variable will
        need to leverage some type of semaphore lock*/
        cs->socketfd = connected_fd;
        fprintf(stdout, "Connected socket for client: %d\n", cs->socketfd);
        fflush(stdout);
        pthread_t handle_connection_th;
        pthread_create(&handle_connection_th, NULL, &connect_send_message_server, cs);
    }

    // close server listening socket
    close(server_fd);

    exit(EXIT_SUCCESS);
}