#!/bin/bash

# make to build client and server
make

touch -a server.log
touch -a server_error.log
cygstart /bin/bash -c \"./server \> server.log 2\> server_error.log \"
touch -a client.log
touch -a client_error.log
cygstart /bin/bash -c \"./client \> client.log 2\> client_error.log \"