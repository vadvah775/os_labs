CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = main

all: $(TARGETS)

main: main.c
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
