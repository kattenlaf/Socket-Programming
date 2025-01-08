#!/bin/bash

# make to build client and server
make

touch -a server.log
cygstart /bin/bash -c \"./server \> server.log \"
touch -a client.log
cygstart /bin/bash -c \"./client \> client.log \"