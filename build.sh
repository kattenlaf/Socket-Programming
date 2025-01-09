#!/bin/bash

# make to build client and server
make

# Cleanup
rm -rf *.log

# Create log files for server
touch -a server.log
touch -a server_error.log
# Create log files for client
touch -a client.log
touch -a client_error.log

# Start both executables with separate terminal windows to serve and send requests
# Also send the stdout and stderr to their own log files
# stderr - 2\>
cygstart /bin/bash -c \"./server \> server.log 2\> server_error.log \"
cygstart /bin/bash -c \"./client \> client.log 2\> client_error.log \"