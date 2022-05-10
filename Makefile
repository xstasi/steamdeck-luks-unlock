CC=gcc
CFLAGS=-Wall -lncurses
PREFIX = /usr

all:
	$(CC) -o deck-unlock main.c $(CFLAGS)

install: all
	install -m 755 deck-unlock $(DESTDIR)$(PREFIX)/bin/
	install -m 755 initramfs-hooks/deck-unlocker $(DESTDIR)$(PREFIX)/share/initramfs-tools/hooks/

clean:
	rm -f deck-unlock
