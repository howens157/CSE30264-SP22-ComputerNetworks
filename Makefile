CC = gcc
CFLAGS = -g -Wall -std=gnu99

all: proj1client proj1server

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

proj1client: proj1client.o
	$(CC) $(CFLAGS) -o $@ $<

proj1server: proj1server.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f proj1client
	rm -f proj1client.o
	rm -f proj1server
	rm -f proj1server.o

