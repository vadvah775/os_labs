CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = mychmod

all: $(TARGETS)

mychmod: mychmod.c
	$(CC) $(CFLAGS) -o mychmod mychmod.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
