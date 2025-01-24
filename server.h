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
    GET,
    PUT,
    POST
} REQUEST_TYPE;

// declarations
ssize_t send_http_ok(int socket, FILE* fptr);
char* open_read_file(FILE* fptr);
void set_fds(fd_set* readfds, int clientfds[], int* maxfd);
void Handle_Client_Request(int socket, char* client_buffer);
bool Handle_Get_Request(int socket, char* resource);
ssize_t Respond_Client(int socket, FILE* fptr);

// definitions

// Summary
// build http ok message and respond to client
ssize_t send_http_ok(int socket, FILE* fptr) {
    char server_buffer[RESPONSE_MSG];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    char current_time[64];
    size_t ret = strftime(current_time, sizeof(current_time), "%c", tm);

    if (fptr == NULL) {
        sprintf(server_buffer, 
            "HTTP/1.1 200 OK\n"
            "Date: %s\n"
            "Server: %s\n"
            "Content-Type: text/html\n"
            "Connection: close\n"
            "\n"
            "<h1>My First Heading</h1>\0", current_time, SERVER_NAME);
        ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
        return bytes_sent;
    } 

    // Get file name from the client buffer request and then search for the resource and send that back to client
    sprintf(server_buffer, 
            "HTTP/1.1 200 OK\n"
            "Date: %s\n"
            "Server: %s\n"
            "Content-Type: text/html\n"
            "Connection: close\n"
            "\n"
            "<h1>My First Heading</h1>\0", current_time, SERVER_NAME);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    return bytes_sent;
}

char* open_read_file(FILE* fptr) {
    char filedata[HTML_FILE_RESPONSE];
    return filedata;
}

// Summary
// Add file descriptors to the fd_set so that server will respond to the client
void set_fds(fd_set* readfds, int clientfds[], int* maxfd) {
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
    REQUEST_TYPE request_type = NULL;
    char request_char[10];
    char resource[MAX_FILENAME_SIZE];
    int position = 0;
    while (client_buffer[i] != '\n') {
        if (request_type == NULL) {
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
        return true;
    } else {
        if (resource[0] == '/') {
            memmove(resource, resource+1, strlen(resource));
            FILE* fptr;
            fptr = fopen(resource, "r");
            ssize_t bytes_sent = Respond_Client(socket, fptr);
        }
    }
}

ssize_t Respond_Client(int socket, FILE* fptr) {
    RESPONSE_CODE response = NULL;
    char server_buffer[RESPONSE_MSG];
    char document[MAX_BUFFER_SIZE];
    char line[MAX_LINE_SIZE];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    char current_time[64];
    size_t ret = strftime(current_time, sizeof(current_time), "%c", tm);

    if (fptr == NULL) {
        response = NOT_FOUND;
        sprintf(server_buffer, 
            "HTTP/1.1 404 NOT FOUND\n"
            "Date: %s\n"
            "Server: %s\n"
            "Connection: close\n"
            "\n", current_time, SERVER_NAME);
        ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
        return bytes_sent;
    } else {
        while(fgets(line, MAX_LINE_SIZE, fptr)) {
            strcat(document, line);
        }
        response = SUCCESS;
        sprintf(server_buffer, 
            "HTTP/1.1 200 OK\n"
            "Date: %s\n"
            "Server: %s\n"
            "Content-Type: text/html\n"
            "Connection: close\n"
            "\n"
            "%s\0", current_time, SERVER_NAME, document);
        ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
        return bytes_sent;
    }
    
}

#endif