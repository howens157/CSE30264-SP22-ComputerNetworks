CC = gcc
CFLAGS = -g -Wall -std=gnu99

all: mycal mycalserver

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycal: client.o
	$(CC) $(CFLAGS) -o $@ $<

mycalserver: server.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f mycal
	rm -f client.o
	rm -f mycalserver
	rm -f server.o

