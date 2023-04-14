CC = ${CROSS_COMPILE}gcc

CROSS_COMPILE ?=

CFLAGS = -g -Wall -Werror

.DEFAULT_GOAL := aesdsocket

all: aesdsocket
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.c

.PHONY: clean
clean: 
	rm -f aesdsocket *.o *.c~ *.h~