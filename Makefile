# the compiler:g++ for C++
CC = g++
CFLAGS = -std=c++11 -c -Wall -pedantic -g

all: main

main: main.o
	$(CC) -o main main.o

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean: 
	rm *.o
