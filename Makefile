CC=gcc
CFLAGS=-I.

all: clean rimuru

rimuru: main.c player.c queue.c
	$(CC) -o rimuru main.c player.c queue.c -lraylib -lGL -lopenal -lm -pthread -ldl 

clean:
	rm -rf ./rimuru || true