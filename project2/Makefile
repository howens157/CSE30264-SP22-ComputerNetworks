CC = gcc
CFLAGS = -g -Wall -std=gnu99
LDFLAGS = `pkg-config --cflags --libs json-glib-1.0`

all: mycalserver

mycalserver:
	$(CC) $(CFLAGS) mycalserver.c -o mycalserver $(LDFLAGS)

clean:
	rm -f mycalserver

