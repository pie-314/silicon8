CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lSDL2

SRC = main.c
OUT = main

all:
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
