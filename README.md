# SpeculativeTomasuloSimulator

Term project for computer architecture, implement a simulator program for speculative tomasulo

### Memmory and Instruction content

I sepearate benchmark to two files that contain instructions and memory contents seperately. Take a look at instr.dat and mem.dat you will know how to make adjustment.

### Environment

The project is developed on Windows Subsystem for Linux (WSL)

### Benchmark in project description (prog.dat)

```
0, 111
8, 14
16, 5
24, 10
100, 2
108, 27
116, 3
124, 8
200, 12

addi R1, R0, 24
addi R2, R0, 124
fld F2, 200(R0)
loop: fld F0, 0(R1)
fmul F0, F0, F2
fld F4, 0(R2)
fadd F0, F0, F4
fsd F0, 0(R2)
addi R1, R1, -8
addi R2, R2, -8
bne R1,$0, loop
```

### My instr.dat

```
addi R1, R0, 24
addi R2, R0, 124
fld F2, 200(R0)
loop: fld F0, 0(R1)
fmul F0, F0, F2
fld F4, 0(R2)
fadd F0, F0, F4
fsd F0, 0(R2)
addi R1, R1, -8
addi R2, R2, -8
bne R1,$0, loop
```

### My mem.dat

```
0, 111
8, 14
16, 5
24, 10
100, 2
108, 27
116, 3
124, 8
200, 12
```

My program pass instr.dat and mem.dat as user input rather than prog.dat.

## How to execute

I made a Makefile to help execute

use **make main** to compile the file

```
make main
```

use **make run** execute the program

```
make run
```

use **make clean** delete the compiled files

```
make clean
```

By changing those parameters in makefile and then **make main** and **make run** can test with different settings.

```
NF=4 #number of fetch instructions pre cycle.
NW=4 #number of instructions can be issued to reservation stations pre cycle.
NR=16 #number of entries in the reorder buffer(ROB)
NB=4  #commit width
```

### Known issue
Can not pass the testcase with nested loop, probabily due to the BTB or physical register allocation.
