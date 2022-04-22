CC = gcc
CFLAGS = -g -Wall -std=gnu99
LDFLAGS = `pkg-config --cflags --libs json-glib-1.0`

.PHONY: mpwordleserver

all: mpwordleserver

mpwordleserver:
	$(CC) $(CFLAGS) mpwordleserver/mpwordleserver.c -o mpwordleserver/mpwordleserver $(LDFLAGS)

clean:
	rm -f mpwordleserver/mpwordleserver

