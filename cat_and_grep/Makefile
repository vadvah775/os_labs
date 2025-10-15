
CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = mycat mygrep

all: $(TARGETS)

mycat: mycat.c
	$(CC) $(CFLAGS) -o mycat mycat.c

mygrep: mygrep.c
	$(CC) $(CFLAGS) -o mygrep mygrep.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean test
