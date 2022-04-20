CC = gcc
CFLAGS = -g -Wall -std=gnu99
LDFLAGS = `pkg-config --cflags --libs json-glib-1.0`

.PHONY: mpwordleserver

all: mpwordleserver mpwordle2

mpwordleserver:
	$(CC) $(CFLAGS) mpwordleserver/mpwordleserver.c -o mpwordleserver/mpwordleserver $(LDFLAGS)

mpwordle2:
	$(CC) $(CFLAGS) mpwordleserver/mpwordle2.c -o mpwordleserver/mpwordle2 $(LDFLAGS)

clean:
	rm -f mpwordleserver/mpwordleserver
	rm -f mpwordleserver/mpwordle2

