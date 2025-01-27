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
#define MAX_CLIENTS 10
#define MAX_LINE_SIZE 100
#define HTTP_VERSION "HTTP/1.1"
#define DEBUG_MSG 512
#define ERROR_MSG 256

typedef struct Connect_Send {
    int socketfd;
    char* message;
    struct sockaddr_in serveraddress;
} Connect_Send;

void close_fd_return(int socketfd) {
    close(socketfd);
    pthread_exit(NULL);
}

void print_stdout(char* msg) {
    fprintf(stdout, msg);
    fflush(stdout);
}

void print_stderr(char* msg) {
    fprintf(stderr, msg);
    fflush(stderr);
}