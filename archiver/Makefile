CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = archiver

all: $(TARGETS)

archiver: archiver.c
	$(CC) $(CFLAGS) -o archiver  archiver.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
