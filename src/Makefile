#include ../Makedefs

# activate optimizations
CFLAGS += -fno-stack-protector -g -Wall -Wextra

# relro with global offset table protection
LDFLAGS += -Wl,-z,relro,-z,now,-z,initfirst $(I386)

# use gcc
CC=gcc

# Make sure we fail if anything fails in a loop
SHELL := /bin/bash -e

# object files
FILES+=main.c helper.c meta.c io.c

LIBDIR=../lib

# we include some kernel stuff as we directly call system calls.
INCLUDEDIR=/lib/modules/$(shell uname -r)/build/arch/x86/include

GEN_DIR=generated/

.PHONY: clean includes

all: domain_manager

domain_manager: *.h $(FILES)
	${CC} ${CFLAGS} $(LIBFLAGS) -I$(INCLUDEDIR) -flto $(FILES) -o domain_manager 


clean:
	rm -f *.o *.lo *.la *~ *.as *.out
	rm -f domain_manager
