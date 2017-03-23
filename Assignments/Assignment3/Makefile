CC=g++
CFLAGS=-pthread -std=c++11

default: all

all: blocking nonblocking

blocking: BLOCKING_LIST.cpp
	$(CC) BLOCKING_LIST.cpp -o BLOCKING_LIST.out $(CFLAGS)

nonblocking: NONBLOCKING_LIST.cpp
	$(CC) NONBLOCKING_LIST.cpp -o NONBLOCKING_LIST.out $(CFLAGS)

clean:
	rm *.out