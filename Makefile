CC = gcc
CFLAGS = -g -Wall -std=gnu99

all: client server

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client: client.o
	$(CC) $(CFLAGS) -o $@ $<

server: server.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f client
	rm -f client.o
	rm -f server
	rm -f server.o

