CC = gcc
CFLAGS = -Wall -Iinclude

SRC = src/main.c src/memory.c src/paging.c src/lru.c src/io.c
OUT = paging

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run:
	./$(OUT)

clean:
	rm -f $(OUT)
