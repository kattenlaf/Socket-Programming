CC=gcc
CFLAGS=-g -ggdb3 -std=c11 -Wall -pedantic -lpthread -ldl -lm
BIN = server
OBJ=sqlite3.o
# https://man7.org/linux/man-pages/man1/gcc.1.html

all: sqlite3.o client
	$(CC) $(CFLAGS) src/server.c sqlite3.o -o src/server

sqlite3.o:
	$(CC) $(CFLAGS) -c src/vendor/sqlite3.c -o $@

client:
	$(CC) $(CFLAGS) src/client.c -o client

clean:
	$(RM) -r $(OBJ)
	$(RM) $(BIN) *.db
