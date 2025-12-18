CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = rwlock

all: $(TARGETS)

rwlock: rwlock.c
	$(CC) $(CFLAGS) -o rwlock rwlock.c -lpthread

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
