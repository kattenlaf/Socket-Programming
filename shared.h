#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 8080
#define MAX_BUFFER_SIZE 2048
#define LOCAL_HOST "127.0.0.1"

typedef struct Connect_Send {
    int socketfd;
    char* message;
    struct sockaddr_in serveraddress;
} Connect_Send;

void close_fd_return(int socketfd) {
    close(socketfd);
    pthread_exit(NULL);
}