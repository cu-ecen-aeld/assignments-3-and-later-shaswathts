CC := gcc

CROSS_COMPILE := aarch64-none-linux-gnu-
CROSS_COMPILE := $(CROSS_COMPILE)$(CC)
#$(info CROSS_COMPILE=$(CROSS_COMPILE))

CFLAGS = -g -Wall

default: writer.o
	$(CC) $(CFLAGS) -o writer writer.c

CROSS_COMPILE: writer.o
	$(CROSS_COMPILE) $(CFLAGS) -o writer writer.c

.PHONY: clean
clean: 
	rm -f writer *.o *.c~ *.h~