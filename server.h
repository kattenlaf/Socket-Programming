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
#define STATUS_LINE 64
#define SUPPORTED_REQUESTS 4

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
    POST,
    DELETE
} REQUEST_TYPE;

typedef enum CONTENT_TYPE {
    APPLICATION_JSON = 0,
    TEXT_HTML,
    TEXT_PLAIN
} CONTENT_TYPE;

const char* REQUEST_TYPES_LIST[] = {
    "GET",
    "PUT",
    "POST",
    "DELETE"
};

const char* CONTENT_TYPE_LIST[] = {
    "application/json",
    "text/html",
    "text/plain"
};

// declarations
// ------------
char* Open_Read_File(FILE* fptr, RESPONSE_CODE* response);
void Set_fds(fd_set* readfds, int clientfds[], int* maxfd);
void Handle_Client_Request(int socket, char* client_buffer);
bool Handle_Get_Request(int socket, char* resource);
ssize_t Respond_Client(int socket, FILE* fptr);
void Build_Response(char server_buf[], char message_body[], RESPONSE_CODE response_code);
REQUEST_TYPE Get_Request_Type(char* request_moniker);
bool Handle_Incorrect_Request(int socket, char error_msg[]);

// definitions
// -----------
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
            break;
        case DEFAULT:
            // respond with 400
            Handle_Incorrect_Request(socket, "");
            break;
    }

    if (result) {
        char debug_log[DEBUG_MSG];
        sprintf(debug_log, "Successfully sent request with resource:{%s} using socket:{%d}\t", resource, socket);
        print_stdout(debug_log);
    } else {
        char err[ERROR_MSG];
        sprintf(err, "Error sending request with resource:{%s} using socket:{%d}\n", resource, socket);
        print_stderr(err);
    }
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
        if (bytes_sent < 0) {
            char error_msg[ERROR_MSG];
            sprintf(error_msg, "Error sending response to client\nRequest resource:{%s}\t Socket Used: {%d}\n", resource, socket);
            print_stderr(error_msg);
        }
        return true;
    } // else respond with expected format for request, perhaps use some default file to request this from client
    Handle_Incorrect_Request(socket, "GET requested has incorrect formatting, please fix!\n");
}

// Summary
// server_buf contains full string that is sent back to the client
// message_body contains string held in message section of http response
// response_code denotes the type of response being sent
void Build_Response(char server_buf[], char message_body[], RESPONSE_CODE response_code) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    char current_time[64];
    size_t ret = strftime(current_time, sizeof(current_time), "%c", tm);
    char status_line[STATUS_LINE];
    sprintf(status_line, "%s ", HTTP_VERSION);
    CONTENT_TYPE content_type;
    switch (response_code) {
        case (SUCCESS):
            strcat(status_line, "200 OK");
            content_type = TEXT_HTML;
            break;
        case (NOT_FOUND):
            strcat(status_line, "404 Not Found");
            sprintf(message_body, "Resource not found\n");
            content_type = TEXT_HTML;
            break;
        case (BAD_REQUEST):
            strcat(status_line, "400 Bad Request");
            strcat(message_body, "\nRequest formatted incorrectly, please fix and reattempt\n");
            content_type = TEXT_PLAIN;
            break;
    }
    sprintf(server_buf, 
        "%s\n"
        "Date: %s\n"
        "Server: %s\n"
        "Content-Type: %s\n"
        "Connection: close\n"
        "\n"
        "%s\0", status_line, current_time, SERVER_NAME, CONTENT_TYPE_LIST[content_type],message_body);
}

REQUEST_TYPE Get_Request_Type(char* request_moniker) {
    int result = -1;
    for (int i = 0; i < SUPPORTED_REQUESTS; i++) {
        result = strcmp(REQUEST_TYPES_LIST[i], request_moniker);
        if (result == 0) {
            return (REQUEST_TYPE) (i + 1);
        }
    }
    // This indicates there was not a match
    return DEFAULT;
}

ssize_t Respond_Client(int socket, FILE* fptr) {
    RESPONSE_CODE response = SUCCESS;
    char server_buffer[RESPONSE_MSG];
    char line[MAX_LINE_SIZE];
    if (fptr == NULL) {
        response = NOT_FOUND;
    }
    char* msg_body = Open_Read_File(fptr, &response);
    Build_Response(server_buffer, msg_body, response);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    free(msg_body);
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

bool Handle_Incorrect_Request(int socket, char error_msg[]) {
    RESPONSE_CODE response = BAD_REQUEST;
    char server_buffer[RESPONSE_MSG];
    char* msg_body = malloc(sizeof(char) * HTML_FILE_RESPONSE);
    sprintf(msg_body, error_msg);
    Build_Response(server_buffer, msg_body, response);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    free(msg_body);
    if (bytes_sent < 0) {
        // some error occurred
        perror("Error responding with bad request to client\n");
        return false;
    }
    return true;
}

#endif