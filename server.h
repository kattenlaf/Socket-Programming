#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>

#ifndef __SERVER_H
#define __SERVER_H

#define PORT 8080
#define MAX_BUFFER_SIZE 2048
#define MAXIMUM_BACKLOG_CONNECTIONS 3
#define LOCAL_HOST "127.0.0.1"

#endif