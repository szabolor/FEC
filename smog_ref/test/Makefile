CC = gcc

CFLAGS = -Wall -Wextra -O3

LDFLAGS = -L../encode/ -L../decode/ -lencref -ldecref -lm

RM = rm
AR = ar


all: test

test: main.c
	$(CC) main.c $(LDFLAGS) -o test

clean: 
	-$(RM) test
