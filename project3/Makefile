CC = gcc
CFLAGS = -g -Wall -std=gnu99
LDFLAGS = `pkg-config --cflags --libs json-glib-1.0`

.PHONY: mycalserver

all: mycalserver

mycalserver:
	$(CC) $(CFLAGS) mycalserver/mycalserver.c -o mycalserver/mycalserver $(LDFLAGS)

clean:
	rm -f mycalserver/mycalserver

