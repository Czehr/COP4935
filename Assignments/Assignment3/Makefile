CC=g++
CFLAGS=-pthread -std=c++11

default: all

all: LFTT

LFTT: LFTT.cpp
	$(CC) LFTT.cpp -o LFTT.out $(CFLAGS)

clean:
	rm *.out