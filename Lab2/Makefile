CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = myls

all: $(TARGETS)

myls: myls.c
	$(CC) $(CFLAGS) -o myls myls.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
