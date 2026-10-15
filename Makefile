CC = gcc
CFLAGS = -O3 -Wall -std=c99

all: scheduler

scheduler: main.c weights.h
	$(CC) $(CFLAGS) main.c -o scheduler -lm

clean:
	rm -f scheduler
