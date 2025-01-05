all: server client

server: main.c
	gcc main.c -o main

client: client.c
	gcc client.c -o client