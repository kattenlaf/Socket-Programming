# HTTP Web Server

Simple web server created using C to understand HTTP requests, sockets and some other posix based libraries;
leveraged Sqlite3 for database operations. 
Will continue to enhance for fun and further learning.

## Features
Server is currently able to process http get, post, put requests and respond accordingly. To try out these requests with the database,
these requests can be made to target the resource pokemon -> i.e GET /pokemon HTTP/1.0

## Tech
Nothing fancy
- [C] - built in C, to learn about sockets and some other backend and lowlevel operations.
- [sqlite3] - for the database

## To try it out
You will need sqlite3 installed and json-c then...
```make``` or compile with ```gcc -I {thisfolder} -I {thisfolder}\src -g -Wall -pedantic -lpthread -ldl -lm src\server.c {thisfolder}\sqlite3.o -ljson-c -o {thisfolder}\server.exe```
```./server```
then making http requests to the server on localhost port 8080 should work.