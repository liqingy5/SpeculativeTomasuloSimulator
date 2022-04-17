# the compiler:g++ for C++
CC = g++
CFLAGS = -std=c++11 -c -Wall -pedantic -g
NF=4 #number of fetch instructions pre cycle.
NW=4 #number of instructions can be issued to reservation stations pre cycle.
NR=16 #number of entries in the reorder buffer(ROB)
NB=4  #number of entries in the Common Data Busses(CDB)
INSFILE = "instr.dat"
MEMFILE = "mem.dat"
all: clean main run

run: 
	./main $(INSFILE) $(MEMFILE) $(NF) $(NW) $(NR) $(NB)

main: main.o simulator.o
	$(CC) -o main main.o simulator.o

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

simulator.o:simulator.cpp
	$(CC) $(CFLAGS) simulator.cpp

clean: 
	rm *.o main simulator
