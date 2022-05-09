CC=gcc
CFLAGS=-Wall -lncurses
all:
	$(CC) -o main main.c $(CFLAGS)
