#include "client.h"

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

    // inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    // https://man7.org/linux/man-pages/man3/inet_pton.3.html
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    int conversion_status = inet_pton(AF_INET, LOCAL_HOST, &server_address.sin_addr);
    if (conversion_status < 0) {
        perror("Address is not valid");
        exit(EXIT_FAILURE);
    } else if (conversion_status == 0) {
        perror("Not a valid character string representing network address");
        exit(EXIT_FAILURE);
    }

    Connect_Send cs;
    cs.serveraddress = server_address;
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * 3);
    for (int i = 0; i < 3; i++) {
        cs.socketfd = socket(AF_INET, SOCK_STREAM, 0);
        if (cs.socketfd < 0) {
            perror("Client Socket Creation Error");
            continue;
        }
        printf("Client file descriptor %d\n", cs.socketfd);
        pthread_create(&threads[i], NULL, &connect_send_message, &cs);
    }

    for (int i = 0; i < 3; i++) {
        (void) pthread_join(threads[i], NULL);
    }

    free(threads);
    printf("Closing client, freeing threads complete\n");
    exit(EXIT_SUCCESS);
}