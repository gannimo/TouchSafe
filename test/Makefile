#include ../Makedefs

# activate optimizations
CFLAGS += -fno-stack-protector -g -Wall

# relro with global offset table protection
LDFLAGS += -Wl,-z,relro,-z,now,-z,initfirst $(I386)

# use gcc
CC=gcc

# Make sure we fail if anything fails in a loop
SHELL := /bin/bash -e

# object files
FILES+=simple_read.c ts_encrypt.c throughput.c

.PHONY: clean includes

all: $(FILES:.c=)

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f *.o *.lo *.la *~ *.as *.out
	rm -f $(FILES:.c=)
