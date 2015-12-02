.PHONY: all clean build test

all: build

build:
	make -C src all

test:
	make -C src clean all
	make -C test clean all

clean:
	make -C src clean
	make -C test clean
