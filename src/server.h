#ifndef __SERVER_H
#define __SERVER_H
#include "shared.h"
#include "helpers/linkedlist.h"
#include <sys/select.h>
#include <time.h>
#include "vendor/cJSON.h"

// server constants
// ----------------
#define MAXIMUM_BACKLOG_CONNECTIONS 3
#define RESPONSE_MSG 4096
#define HTML_FILE_RESPONSE 2048
#define MAX_FILENAME_SIZE 255
#define SERVER_NAME "TimboG/Server"
#define STATUS_LINE 64
#define SUPPORTED_REQUESTS 4
#define AUTHENTICATION "Authorization"
#define MAX_TOKEN_SIZE 255
#define CLIENT_TOKEN "1234"

// server statuses etc
// -------------------
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

typedef struct Token {
    char* value;
    bool valid;
} Token;

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

// Summary
// primary context for hold server state at a given time
typedef struct Server_Context {
    List* message_bus;
    RESPONSE_CODE response;
    REQUEST_TYPE request_type;
    CONTENT_TYPE content_type;
    char* status_line;
} Server_Context;

// declarations
// ------------
char* OpenReadFile(FILE* fptr, RESPONSE_CODE* response);
void Setfds(fd_set* readfds, int clientfds[], int* maxfd);
void HandleClientRequest(int socket, char* client_buffer, Server_Context* context);
bool HandleGetRequest(int socket, char* resource, char* client_buffer, Server_Context* context);
bool HandlePostRequest(int socket, char* resource, char* client_buffer, Server_Context* context);
ssize_t RespondClient(int socket, FILE* fptr, Server_Context* context);
void BuildResponse(char server_buf[], char message_body[], RESPONSE_CODE response_code, Server_Context* context);
REQUEST_TYPE GetRequestType(char* request_moniker);
bool HandleIncorrectRequest(int socket, char error_msg[], Server_Context* context);
bool CheckAccess(Server_Context* context, char* resource, char* client_buffer, int check_mode);
Token ParseToken(char* authorization);
bool IsClientToken(Token token);
Server_Context* InitContext();

// definitions
// -----------
Server_Context* InitContext() {
    Server_Context* context = malloc(sizeof(Server_Context));
    context->request_type = DEFAULT;
    context->response = SUCCESS;
    context->message_bus = InitList();
    context->status_line = malloc(sizeof(char) * STATUS_LINE);
    context->content_type = TEXT_PLAIN;
    return context;
}


// Summary
// Add file descriptors to the fd_set so that server will respond to the client
void Setfds(fd_set* readfds, int clientfds[], int* maxfd) {
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
void HandleClientRequest(int socket, char* client_buffer, Server_Context* context) {
    int i = 0;
    int j = 0;
    char request_char[10];
    char resource[MAX_FILENAME_SIZE];
    int position = 0;
    while (client_buffer[i] != '\n') {
        if (context->request_type == DEFAULT) {
            if (client_buffer[i] == ' ') {
                request_char[i] = '\0';
                context->request_type = GetRequestType(request_char);
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

    // result is an indication that the response was sent successfully
    bool result = true;
    switch (context->request_type) {
        case GET:
            result = HandleGetRequest(socket, resource, client_buffer, context);
            break;
        case POST:
            result = HandlePostRequest(socket, resource, client_buffer, context);
        case DEFAULT:
            // respond with 400
            HandleIncorrectRequest(socket, "", context);
            break;
    }

    char server_log[SERVER_MSG];
    if (result) {
        DumpContextMessages(context->message_bus, server_log, LOG);
        print_stdout(server_log);
    } else {
        DumpContextMessages(context->message_bus, server_log, ERROR);
        print_stderr(server_log);
    }
}

bool HandleGetRequest(int socket, char* resource, char* client_buffer, Server_Context* context) {
    int result = strcmp("/", resource);
    if (result == 0) {
        resource = strcat(resource, "index.html"); // return index.html by default
    }
    if (resource[0] == '/') {
        memmove(resource, resource+1, strlen(resource));
        FILE* fptr;
        if (access(resource, F_OK) == 0) {
            // check if user has permission
            if (CheckAccess(context, resource, client_buffer, R_OK)) {
                fptr = fopen(resource, "r");
            } else {
                context->response = UNAUTHORIZED;
                fptr = NULL;
            }
        } else {
            fptr = NULL;
        }
        ssize_t bytes_sent = RespondClient(socket, fptr, context);
        if (bytes_sent < 0) {
            char server_log[SERVER_MSG];
            sprintf(server_log, "Error sending response to client\nRequest resource:{%s}\t Socket Used: {%d}\n", resource, socket);
            AddContextMessage(context->message_bus, server_log, ERROR);
            return false;
        }
        return true;
    } 
    // else respond with expected format for request, perhaps use some default file to request this from client
    HandleIncorrectRequest(socket, "GET requested has incorrect formatting, please fix!\n", context);
}

bool HandlePostRequest(int socket, char* resource, char* client_buffer, Server_Context* context) {
    // Introduce tomorrow
    // Parse json file from request to enter into sqlite database
    // find what table they are writing the data to
    return true;
}

bool HandleIncorrectRequest(int socket, char error_msg[], Server_Context* context) {
    RESPONSE_CODE response = BAD_REQUEST;
    char server_buffer[RESPONSE_MSG];
    char* msg_body = malloc(sizeof(char) * HTML_FILE_RESPONSE);
    sprintf(msg_body, error_msg);
    BuildResponse(server_buffer, msg_body, response, context);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    free(msg_body);
    if (bytes_sent < 0) {
        // some error occurred
        perror("Error responding with bad request to client\n");
        return false;
    }
    return true;
}

// Summary
// Builds response to send to client
// Params
// server_buf contains full string that is sent back to the client
// message_body contains string held in message section of http response
// response_code denotes the type of response being sent
void BuildResponse(char server_buf[], char message_body[], RESPONSE_CODE response_code, Server_Context* context) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    char current_time[64];
    size_t ret = strftime(current_time, sizeof(current_time), "%c", tm);
    sprintf(context->status_line, "%s ", HTTP_VERSION);
    CONTENT_TYPE content_type;
    switch (response_code) {
        case (SUCCESS):
            strcat(context->status_line, "200 OK");
            content_type = TEXT_HTML;
            break;
        case (NOT_FOUND):
            strcat(context->status_line, "404 Not Found");
            AddContextMessage(context->message_bus, "Resource not found\n", ERROR);
            content_type = TEXT_HTML;
            break;
        case (BAD_REQUEST):
            strcat(context->status_line, "400 Bad Request");
            AddContextMessage(context->message_bus, "Request formatted incorrectly, please fix and reattempt\n", ERROR);
            content_type = TEXT_PLAIN;
            break;
        case (UNAUTHORIZED):
            strcat(context->status_line, "401 Unauthorized");
            AddContextMessage(context->message_bus, "User does not have access to resource\n", ERROR);
            content_type = TEXT_PLAIN;
            break;
    }
    DumpContextMessages(context->message_bus, message_body, ALL);
    sprintf(server_buf, 
        "%s\n"
        "Date: %s\n"
        "Server: %s\n"
        "Content-Type: %s\n"
        "Connection: close\n"
        "\n"
        "%s\0", context->status_line, current_time, SERVER_NAME, CONTENT_TYPE_LIST[content_type],message_body);
}

REQUEST_TYPE GetRequestType(char* request_moniker) {
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

ssize_t RespondClient(int socket, FILE* fptr, Server_Context* context) {
    char server_buffer[RESPONSE_MSG];
    char line[MAX_LINE_SIZE];
    char* msg_body = OpenReadFile(fptr, &context->response);
    BuildResponse(server_buffer, msg_body, context->response, context);
    ssize_t bytes_sent = send(socket, server_buffer, strlen(server_buffer), 0);
    free(msg_body);
    return bytes_sent;
}

// Open and read file to send back to client
char* OpenReadFile(FILE* fptr, RESPONSE_CODE* response) {
    char* body = malloc(sizeof(char) * HTML_FILE_RESPONSE);
    if (fptr == NULL) {
        if (*response == SUCCESS) {
            *response = NOT_FOUND;
        }
    } else {
        char line[MAX_LINE_SIZE];
        while(fgets(line, MAX_LINE_SIZE, fptr)) {
            strcat(body, line);
        }
    }
    return body;
}

bool CheckAccess(Server_Context* context, char* resource, char* client_buffer, int check_mode) {
    // Check if resource permissions allowed, also maybe implement a client auth system.
    const char* current_line = client_buffer;
    Token token;
    token.valid = false;
    bool authHeaderExists = false;
    if (check_mode == R_OK && access(resource, check_mode) == 0) {
        // parse client_buffer for details, authenticate user and then allow read
        while(current_line) {
            const char* next_line = strchr(current_line, '\n');
            int current_line_len = next_line ? (next_line-current_line) : strlen(current_line);
            char* temp_current_line = (char*)malloc(current_line_len+1);
            if (temp_current_line) {
                memcpy(temp_current_line, current_line, current_line_len);
                temp_current_line[current_line_len] = '\0';
                // Check for the authorization header in temp
                // Authorization: Bearer 123456
                char* authorization = strstr(temp_current_line, AUTHENTICATION);
                if (authorization) {
                    authHeaderExists = true;
                    token = ParseToken(authorization);
                    free(temp_current_line);
                    break;
                }
                free(temp_current_line);
            }  
            current_line = next_line ? (next_line+1) : NULL;
        }
        if (!authHeaderExists) {
            AddContextMessage(context->message_bus, "Client error: Authorization header is not present\n", ERROR);
        }
    }
    return IsClientToken(token);
}

Token ParseToken(char* authorization) {
    int spaces = 2;
    char token_string[MAX_TOKEN_SIZE];
    Token token;
    token.valid = false;
    int token_pos = 0;
    int length = strlen(authorization);
    for (int i = 0; i < strlen(authorization); i++) {
        if (spaces == 0) {
            token_string[token_pos] = authorization[i];
            token_pos++;
        }
        if (authorization[i] == ' ') {
            spaces--;
        }
    }
    if (spaces == 0) {
        token.valid = true;
        token.value = token_string;
    }
    token.value[strcspn(token.value, "\r\n")] = 0;
    return token;
}

bool IsClientToken(Token token) {
    if (!token.valid) {
        return token.valid;
    }
    // maybe bolster this somehow but for now will just do a simple check
    int result = strcmp(token.value, CLIENT_TOKEN);
    return result == 0;
}

#endif