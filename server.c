#include "server.h" // https://man7.org/linux/man-pages/man2/listen.2.html

int main(void) {
    int master_socket, new_connected_fd; // File descriptor and new socket created on acceptance
    struct sockaddr_in address;
    int opt, i;
    socklen_t addrlen = sizeof(address);
    char* message = "Hello from server\n";
    char buffer[MAX_BUFFER_SIZE] = {0};
    int clientfds[MAX_CLIENTS] = {0};
    fd_set readfds;
    size_t dataread;

    /* Set up socket and bind to port */
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind to local host 8080
    if (bind(master_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    // Server socket only needs to listen once
    if (listen(master_socket, MAX_CLIENTS) < 0) {
        perror("listening failure");
        exit(EXIT_FAILURE);
    }

    // initialize the client file descriptors to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        clientfds[i] = 0;
    }

    int maxfd;
    int sd = 0;
    int fds_to_read;

    while (true) {
        fprintf(stdout, "Webserver is waiting to accept request...\n");
        fflush(stdout);
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clientfds[i];
            FD_SET(sd, &readfds);
            if (sd > maxfd) {
                maxfd = sd;
            }
        }

        if (sd > maxfd) {
            maxfd = sd;
        }

        // https://linux.die.net/man/2/select, timeout being NULL means select can block indefinitely
        fds_to_read = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (fds_to_read == 0) {
            fprintf(stdout, "no file descriptors to read\n");
            fflush(stdout);
            continue;
        } else if (fds_to_read < 0) {
            perror("Server select error\n");
            continue;
        }

        
        if (FD_ISSET(master_socket, &readfds)) {
            // client_fd = accept(master_socket, (struct sockaddr*)NULL, NULL);
            new_connected_fd = accept(master_socket, (struct sockaddr*)&address, &addrlen);
            if (new_connected_fd < 0) {
                perror("client accept error!\n");
                continue;
            }
        }

        if (send(new_connected_fd, message, strlen(message), 0) != strlen(message)) {
            perror("Failure sending message to client\n");
            continue;
        }
        print_stdout("Message successfully sent!\n");
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clientfds[i] == 0) {
                clientfds[i] = new_connected_fd;
                print_stdout("Adding to client file descriptors\n");
                break;
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clientfds[i];

            if (FD_ISSET(sd, &readfds)) {
                dataread = read(sd, buffer, MAX_BUFFER_SIZE);
                // 0 bytes meant no data was read, that means client was added but,
                // disconnected eventually as no data is available to be read, close the socket as well
                if (dataread == 0) {
                    print_stdout("client disconnect\n");
                    close(sd);
                } else if (dataread < 0) {
                    perror("error reading from client\n");
                    continue;
                } else {
                    print_stdout("message from client\n");
                    print_stdout(buffer);
                }
            }
        }
    }

    // close server master listening socket
    close(master_socket);

    exit(EXIT_SUCCESS);
}