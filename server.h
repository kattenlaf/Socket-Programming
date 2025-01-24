#include "shared.h"
#include <sys/select.h>
#include <time.h>

#ifndef __SERVER_H
#define __SERVER_H
#define MAXIMUM_BACKLOG_CONNECTIONS 3
#define RESPONSE_MSG 4096
#define HTML_FILE_RESPONSE 2048
#define MAX_FILENAME_SIZE 255
#define SERVER_NAME "TimboG/Server"

typedef enum RESPONSE_CODE {
    SUCCESS = 200,
    CREATED,
    ACCEPTED,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    UNAUTHORIZED,
    FORBIDDEN = 403,
    NOT_FOUND
} RESPONSE_CODE;

typedef enum REQUEST_TYPE {
    DEFAULT,
    GET,
    PUT,
    POST
} REQUEST_TYPE;

// declarations
ssize_t Send_Http_OK(int socket, FILE* fptr);
char* Open_Read_File(FILE* fptr, RESPONSE_CODE* response);
void Set_fds(fd_set* readfds, int clientfds[], int* maxfd);
void Handle_Client_Request(int socket, char* client_buffer);
bool Handle_Get_Request(int socket, char* resource);
ssize_t Respond_Client(int socket, FILE* fptr);
void Build_Response(char server_buf[], char message_body[]);
REQUEST_TYPE Get_Request_Type(char* request_moniker);

// definitions

// Summary
// build http ok message and respond to client
ssize_t Send_Http_OK(int socket, FILE* fptr) {
    char server_buffer[RESPONSE_MSG];
    char msg_body[MAX_BUFFER_SIZE];
    Build_Response(server_buffer, msg_body);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    return bytes_sent;
}

void Build_Response(char server_buf[], char message_body[]) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    char current_time[64];
    size_t ret = strftime(current_time, sizeof(current_time), "%c", tm);
    sprintf(server_buf, 
        "%s 200 OK\n"
        "Date: %s\n"
        "Server: %s\n"
        "Content-Type: text/html\n"
        "Connection: close\n"
        "\n"
        "%s\0", HTTP_VERSION, current_time, SERVER_NAME, message_body);
}

// Summary
// Add file descriptors to the fd_set so that server will respond to the client
void Set_fds(fd_set* readfds, int clientfds[], int* maxfd) {
    int sd;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sd = clientfds[i];
        FD_SET(sd, readfds);
        if (sd > *maxfd) {
            *maxfd = sd;
        }
    }
    if (sd > *maxfd) {
        *maxfd = sd;
    }
}

// Summary
// Determine what type of request it is, if it is a GET request, parse in that format
void Handle_Client_Request(int socket, char* client_buffer) {
    int i = 0;
    int j = 0;
    REQUEST_TYPE request_type = DEFAULT;
    char request_char[10];
    char resource[MAX_FILENAME_SIZE];
    int position = 0;
    while (client_buffer[i] != '\n') {
        if (request_type == DEFAULT) {
            if (client_buffer[i] == ' ') {
                request_char[i] = '\0';
                request_type = Get_Request_Type(request_char);
            } else {
                request_char[i] = client_buffer[i];
            }
        } else {
            if (client_buffer[i] == ' ') {
                // file name would have been build here, can stop loop
                resource[j] = '\0';
                break;
            }
            else {
                resource[j] = client_buffer[i];
            }
            j++;
        }
        i++;
    }

    bool result = true;
    switch (request_type) {
        case GET:
            result = Handle_Get_Request(socket, resource);
    }
}

REQUEST_TYPE Get_Request_Type(char* request_moniker) {
    int result = -1;
    result = strcmp("GET", request_moniker);
    if (result == 0) {
        return GET;
    }

    // Return get by default
    return GET;
}

bool Handle_Get_Request(int socket, char* resource) {
    int result = strcmp("/", resource);
    if (result == 0) {
        resource = strcat(resource, "index.html"); // return index.html by default
    }
    if (resource[0] == '/') {
        memmove(resource, resource+1, strlen(resource));
        FILE* fptr;
        fptr = fopen(resource, "r");
        ssize_t bytes_sent = Respond_Client(socket, fptr);
        return true;
    }
}

ssize_t Respond_Client(int socket, FILE* fptr) {
    RESPONSE_CODE response = SUCCESS;
    char server_buffer[RESPONSE_MSG];
    char document[MAX_BUFFER_SIZE];
    char line[MAX_LINE_SIZE];
    if (fptr == NULL) {
        response = NOT_FOUND;
    }
    char* msg_body = Open_Read_File(fptr, &response);
    Build_Response(server_buffer, msg_body);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    return bytes_sent;
}

// Open and read file to send back to client
char* Open_Read_File(FILE* fptr, RESPONSE_CODE* response) {
    char* body = malloc(sizeof(char) * HTML_FILE_RESPONSE);
    char line[MAX_LINE_SIZE];
    if (fptr == NULL) {
        *response = NOT_FOUND;
    } else {
        while(fgets(line, MAX_LINE_SIZE, fptr)) {
            strcat(body, line);
        }
    }
    return body;
}

#endif