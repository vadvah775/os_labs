CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = task1 sender receiver

all: $(TARGETS)

task1: task1.c
	$(CC) $(CFLAGS) -o task1 task1.c -lpthread

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
